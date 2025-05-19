package handlers

import (
	"database/sql"
	"drift-backend/internal/middleware"
	"encoding/json"
	"net/http"
	"time"
)

// Add appointment types
type CreateScheduleRequest struct {
	FirstName     string `json:"first_name"`
	LastName      string `json:"last_name"`
	AvailableDate string `json:"available_date"`
	StartTime     string `json:"start_time"`
	EndTime       string `json:"end_time"`
}

type DeleteScheduleRequest struct {
	FirstName     string `json:"first_name"`
	LastName      string `json:"last_name"`
	AvailableDate string `json:"available_date"`
	StartTime     string `json:"start_time"`
}

// Get schedule type
type ScheduleResponse struct {
	ScheduleID    int    `json:"schedule_id"`
	AvailableDate string `json:"available_date"`
	StartTime     string `json:"start_time"`
	EndTime       string `json:"end_time"`
	IsBooked      bool   `json:"is_booked"`
	FirstName     string `json:"first_name"`
	LastName      string `json:"last_name"`
	UserID        *int   `json:"user_id"`
}

// Book schedule type
type BookScheduleRequest struct {
	ScheduleID int  `json:"schedule_id"`
	Book       bool `json:"book"`
}

// Add appointment
func (h *Handler) HandleCreateSchedule(w http.ResponseWriter, r *http.Request) {
	var req CreateScheduleRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Некорректный JSON", http.StatusBadRequest)
		return
	}

	var instructorID int
	err := h.DB.QueryRow(`
		SELECT instructor_id FROM instructors
		WHERE first_name = $1 AND last_name = $2
	`, req.FirstName, req.LastName).Scan(&instructorID)
	if err != nil {
		http.Error(w, "Инструктор не найден", http.StatusNotFound)
		return
	}

	_, err = h.DB.Exec(`
		INSERT INTO schedule (instructor_id, available_date, start_time, end_time, is_booked)
		VALUES ($1, $2, $3, $4, false)
	`, instructorID, req.AvailableDate, req.StartTime, req.EndTime)
	if err != nil {
		http.Error(w, "Не удалось создать слот", http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]string{
		"message": "Слот успешно создан",
	})
}

// Delete appointment
func (h *Handler) HandleDeleteSchedule(w http.ResponseWriter, r *http.Request) {
	var req DeleteScheduleRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Некорректный JSON", http.StatusBadRequest)
		return
	}

	result, err := h.DB.Exec(`
		DELETE FROM schedule
		WHERE instructor_id = (
			SELECT instructor_id FROM instructors
			WHERE first_name = $1 AND last_name = $2
		)
		AND available_date = $3
		AND start_time = $4
	`, req.FirstName, req.LastName, req.AvailableDate, req.StartTime)
	if err != nil {
		http.Error(w, "Не удалось удалить слот", http.StatusInternalServerError)
		return
	}

	rowsAffected, _ := result.RowsAffected()
	if rowsAffected == 0 {
		http.Error(w, `{"error":"Слот не найден"}`, http.StatusNotFound)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.Write([]byte(`{"message":"Слот удалён"}`))
}

// Get schedule
func (h *Handler) HandleGetSchedule(w http.ResponseWriter, r *http.Request) {
	rows, err := h.DB.Query(`
		SELECT s.schedule_id, s.available_date, s.start_time, s.end_time, s.is_booked,
		       i.first_name, i.last_name, s.user_id
		FROM schedule s
		JOIN instructors i ON s.instructor_id = i.instructor_id
	`)
	if err != nil {
		http.Error(w, "Не удалось получить расписание", http.StatusInternalServerError)
		return
	}
	defer rows.Close()

	var results []ScheduleResponse
	for rows.Next() {
		var (
			scheduleID    int
			availableDate time.Time
			startTime     time.Time
			endTime       time.Time
			isBooked      bool
			firstName     string
			lastName      string
			userID        sql.NullInt64
		)

		if err := rows.Scan(&scheduleID, &availableDate, &startTime, &endTime, &isBooked, &firstName, &lastName, &userID); err != nil {
			http.Error(w, "Ошибка чтения слотов", http.StatusInternalServerError)
			return
		}

		res := ScheduleResponse{
			ScheduleID:    scheduleID,
			AvailableDate: availableDate.Format("2006-01-02"),
			StartTime:     startTime.Format("15:04:05"),
			EndTime:       endTime.Format("15:04:05"),
			IsBooked:      isBooked,
			FirstName:     firstName,
			LastName:      lastName,
		}
		if userID.Valid {
			uid := int(userID.Int64)
			res.UserID = &uid
		}

		results = append(results, res)
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(results)
}

// Book apointment
func (h *Handler) HandleBookSchedule(w http.ResponseWriter, r *http.Request) {
	roleVal := r.Context().Value(middleware.ContextRole)
	userIDVal := r.Context().Value(middleware.ContextUserID)

	role, ok := roleVal.(string)
	if !ok || (role != "user" && role != "admin") {
		http.Error(w, "Недостаточно прав", http.StatusUnauthorized)
		return
	}

	userID, ok := userIDVal.(int)
	if !ok {
		http.Error(w, "Некорректный идентификатор пользователя", http.StatusInternalServerError)
		return
	}

	var req BookScheduleRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Некорректный JSON", http.StatusBadRequest)
		return
	}

	if req.Book {
		var instructorID int
		var orderDate string
		err := h.DB.QueryRow(`
			SELECT instructor_id, available_date
			FROM schedule
			WHERE schedule_id = $1
		`, req.ScheduleID).Scan(&instructorID, &orderDate)
		if err != nil {
			http.Error(w, "Слот не найден", http.StatusNotFound)
			return
		}

		tx, err := h.DB.Begin()
		if err != nil {
			http.Error(w, "Ошибка транзакции", http.StatusInternalServerError)
			return
		}

		res, err := tx.Exec(`
			UPDATE schedule
			SET is_booked = true, user_id = $1
			WHERE schedule_id = $2 AND (user_id IS NULL OR user_id = $1)
		`, userID, req.ScheduleID)
		if err != nil {
			tx.Rollback()
			http.Error(w, "Не удалось забронировать слот", http.StatusInternalServerError)
			return
		}

		rows, _ := res.RowsAffected()
		if rows == 0 {
			tx.Rollback()
			http.Error(w, "Слот уже забронирован другим пользователем", http.StatusForbidden)
			return
		}

		_, err = tx.Exec(`
			INSERT INTO orders (user_id, instructor_id, order_date, status, total_price)
			VALUES ($1, $2, $3, 'забронировано', 0)
		`, userID, instructorID, orderDate)
		if err != nil {
			tx.Rollback()
			http.Error(w, "Не удалось создать заказ", http.StatusInternalServerError)
			return
		}

		tx.Commit()
		json.NewEncoder(w).Encode(map[string]string{"message": "Слот успешно забронирован"})
		return
	}

	tx, err := h.DB.Begin()
	if err != nil {
		http.Error(w, "Ошибка транзакции", http.StatusInternalServerError)
		return
	}

	res, err := tx.Exec(`
		UPDATE schedule
		SET is_booked = false, user_id = NULL
		WHERE schedule_id = $1 AND user_id = $2
	`, req.ScheduleID, userID)
	if err != nil {
		tx.Rollback()
		http.Error(w, "Не удалось отменить бронь", http.StatusInternalServerError)
		return
	}

	rows, _ := res.RowsAffected()
	if rows == 0 {
		tx.Rollback()
		http.Error(w, "Вы не можете отменить чужую бронь", http.StatusForbidden)
		return
	}

	_, err = tx.Exec(`
		UPDATE orders
		SET status = 'отменено'
		WHERE user_id = $1 AND order_date = (
			SELECT available_date FROM schedule WHERE schedule_id = $2
		)
	`, userID, req.ScheduleID)
	if err != nil {
		tx.Rollback()
		http.Error(w, "Не удалось обновить заказ", http.StatusInternalServerError)
		return
	}

	tx.Commit()
	json.NewEncoder(w).Encode(map[string]string{"message": "Бронь успешно отменена"})
}
