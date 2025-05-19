package handlers

import (
	"encoding/json"
	"net/http"
)

// Get Services type
type Service struct {
	ServiceID       int     `json:"service_id"`
	ServiceName     string  `json:"service_name"`
	Description     string  `json:"description"`
	Price           float64 `json:"price"`
	DurationMinutes int     `json:"duration_minutes"`
}

// Get Services
func (h *Handler) HandleGetServices(w http.ResponseWriter, r *http.Request) {
	rows, err := h.DB.Query(`
		SELECT service_id, service_name, description, price, duration_minutes
		FROM services
	`)
	if err != nil {
		http.Error(w, `{"error":"Не удалось получить список услуг"}`, http.StatusInternalServerError)
		return
	}
	defer rows.Close()

	var services []Service
	for rows.Next() {
		var s Service
		if err := rows.Scan(&s.ServiceID, &s.ServiceName, &s.Description, &s.Price, &s.DurationMinutes); err != nil {
			http.Error(w, `{"error":"Ошибка при чтении данных об услуге"}`, http.StatusInternalServerError)
			return
		}
		services = append(services, s)
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(services)
}
