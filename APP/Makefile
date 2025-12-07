# Makefile for Smart Garage Application Unit Tests
# Usage: make test

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O0 -DUNIT_TEST
TARGET = test_app
SOURCE = test_app.c
INCLUDES = -I.

# Mock FreeRTOS and HAL headers
MOCK_DEFS = -DUNIT_TEST

# Default target
all: $(TARGET)

# Build the test executable
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(INCLUDES) $(MOCK_DEFS) -o $(TARGET) $(SOURCE)

# Run tests
test: $(TARGET)
	@echo "Running Smart Garage Unit Tests..."
	@echo "=================================="
	./$(TARGET)
	@echo ""
	@echo "Test execution completed."

# Run tests with verbose output
test-verbose: $(TARGET)
	@echo "Running Smart Garage Unit Tests (Verbose)..."
	@echo "============================================"
	./$(TARGET) 2>&1 | tee test_results.log
	@echo ""
	@echo "Test results saved to test_results.log"

# Check memory leaks (requires valgrind)
test-memory: $(TARGET)
	@echo "Running memory leak detection..."
	@echo "================================"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Generate coverage report (requires gcov)
coverage: CFLAGS += --coverage
coverage: clean $(TARGET)
	@echo "Running tests with coverage..."
	./$(TARGET)
	@echo "Generating coverage report..."
	gcov $(SOURCE)
	@echo "Coverage report generated: $(SOURCE).gcov"

# Clean build artifacts
clean:
	rm -f $(TARGET) *.o *.gcov *.gcda *.gcno test_results.log

# Help target
help:
	@echo "Available targets:"
	@echo "  all         - Build the test executable"
	@echo "  test        - Run the unit tests"
	@echo "  test-verbose- Run tests with verbose output"
	@echo "  test-memory - Run tests with memory leak detection"
	@echo "  coverage    - Run tests with coverage analysis"
	@echo "  clean       - Clean build artifacts"
	@echo "  help        - Show this help message"

.PHONY: all test test-verbose test-memory coverage clean help