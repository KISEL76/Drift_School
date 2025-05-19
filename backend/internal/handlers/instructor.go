package handlers

import (
	"encoding/json"
	"log"
	"net/http"
)

// Add instructors types
type Instructor struct {
	InstructorID   int    `json:"instructor_id"`
	FirstName      string `json:"first_name"`
	LastName       string `json:"last_name"`
	Specialization string `json:"specialization"`
	PhoneNumber    string `json:"phone_number"`
	Email          string `json:"email"`
}

type NewInstructorRequest struct {
	FirstName      string `json:"first_name"`
	LastName       string `json:"last_name"`
	Specialization string `json:"specialization"`
	PhoneNumber    string `json:"phone_number"`
	Email          string `json:"email"`
}

type NewInstructorResponse struct {
	Message string `json:"message,omitempty"`
	Error   string `json:"error,omitempty"`
}

// Fire instructors types
type FireInstructorRequest struct {
	InstructorID int `json:"instructor_id"`
}

type ErrorResponse struct {
	Error string `json:"error"`
}

type SuccessResponse struct {
	Message string `json:"message"`
}

// Add instructor
func (h *Handler) HandleAddInstructor(w http.ResponseWriter, r *http.Request) {
	var req NewInstructorRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(NewInstructorResponse{Error: "Некорректный запрос"})
		return
	}

	var id int
	err := h.DB.QueryRow(`
		INSERT INTO instructors (
			first_name, 
			last_name, 
			specialization, 
			phone_number, 
			email
		)
		VALUES ($1, $2, $3, $4, $5)
		RETURNING instructor_id
	`, req.FirstName, req.LastName, req.Specialization, req.PhoneNumber, req.Email).Scan(&id)

	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(NewInstructorResponse{Error: "Не удалось добавить инструктора"})
		return
	}

	json.NewEncoder(w).Encode(NewInstructorResponse{
		Message: "Инструктор успешно добавлен",
	})
}

// Get instructors
func (h *Handler) HandleGetInstructors(w http.ResponseWriter, r *http.Request) {
	rows, err := h.DB.Query(`
		SELECT 
			instructor_id, 
			first_name, 
			last_name, 
			specialization, 
			phone_number, 
			email
		FROM instructors
	`)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(ErrorResponse{Error: "Не удалось получить инструкторов"})
		return
	}
	defer rows.Close()

	var instructors []Instructor
	for rows.Next() {
		var instr Instructor
		if err := rows.Scan(
			&instr.InstructorID,
			&instr.FirstName,
			&instr.LastName,
			&instr.Specialization,
			&instr.PhoneNumber,
			&instr.Email,
		); err != nil {
			continue
		}
		instructors = append(instructors, instr)
	}

	json.NewEncoder(w).Encode(instructors)
}

// Fire instructor
func (h *Handler) HandleFireInstructor(w http.ResponseWriter, r *http.Request) {
	var req FireInstructorRequest

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(ErrorResponse{Error: "Некорректное тело запроса"})
		return
	}

	res, err := h.DB.Exec(`
		DELETE FROM instructors 
		WHERE instructor_id = $1
	`, req.InstructorID)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(ErrorResponse{Error: "Ошибка при удалении инструктора"})
		return
	}

	affected, err := res.RowsAffected()
	if err != nil {
		log.Println("Ошибка при получении количества строк:", err)
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(ErrorResponse{Error: "Не удалось определить результат операции"})
		return
	}

	if affected == 0 {
		w.WriteHeader(http.StatusNotFound)
		json.NewEncoder(w).Encode(ErrorResponse{Error: "Инструктор не найден"})
		return
	}

	json.NewEncoder(w).Encode(SuccessResponse{Message: "Инструктор успешно удалён"})
}
