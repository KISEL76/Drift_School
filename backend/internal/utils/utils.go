package utils

import (
	"context"
	"errors"

	"drift-backend/internal/middleware"
)

func ExtractUserIDFromContext(ctx context.Context) (int, error) {
	value := ctx.Value(middleware.ContextUserID)
	if value == nil {
		return 0, errors.New("user_id not found in context")
	}

	userID, ok := value.(int)
	if !ok {
		return 0, errors.New("user_id is not an integer")
	}

	return userID, nil
}
