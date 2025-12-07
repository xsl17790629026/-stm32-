#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "app.h"

// Integration tests for the Smart Garage Application
// These tests focus on integration scenarios between different components

// Mock definitions (same as in test_app.c)
typedef void *TaskHandle_t;
typedef void *QueueHandle_t;
typedef unsigned int UBaseType_t;
typedef unsigned int TickType_t;

#define pdPASS 1
#define portMAX_DELAY 0xFFFFFFFF

typedef struct {
    void *Instance;
} UART_HandleTypeDef;

typedef struct {
    void *Instance;
} GPIO_TypeDef;

typedef enum {
    GPIO_PIN_5 = 0x0020U,
    GPIO_PIN_13 = 0x2000U
} GPIO_Pin_TypeDef;

typedef enum {
    HAL_OK = 0x00U,
    HAL_ERROR = 0x01U,
    HAL_BUSY = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

UART_HandleTypeDef huart1 = {0};
GPIO_TypeDef *GPIOB = (GPIO_TypeDef*)0x40010800;
GPIO_TypeDef *GPIOC = (GPIO_TypeDef*)0x40011000;

// Global test tracking
static int hal_uart_transmit_count = 0;
static int hal_uart_receive_it_count = 0;
static int hal_gpio_toggle_pin_count = 0;
static uint8_t last_transmitted_data[256];
static uint16_t last_transmitted_length = 0;

// Mock implementations
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    (void)huart; (void)Timeout;
    hal_uart_transmit_count++;
    if (Size <= sizeof(last_transmitted_data)) {
        memcpy(last_transmitted_data, pData, Size);
        last_transmitted_length = Size;
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) {
    (void)huart; (void)pData; (void)Size;
    hal_uart_receive_it_count++;
    return HAL_OK;
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    (void)GPIOx; (void)GPIO_Pin;
    hal_gpio_toggle_pin_count++;
}

void OLED_Init(void) {}
void OLED_Clear(void) {}
void OLED_Update(void) {}
void OLED_ShowChinese(uint8_t x, uint8_t y, const char *str) { (void)x; (void)y; (void)str; }
void OLED_ShowNum(uint8_t x, uint8_t y, uint16_t num, uint8_t len, uint8_t size) { (void)x; (void)y; (void)num; (void)len; (void)size; }
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size) { (void)x; (void)y; (void)str; (void)size; }

BaseType_t xTaskCreate(void (*pvTaskCode)(void *), const char *pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, TaskHandle_t *pxCreatedTask) {
    (void)pvTaskCode; (void)pcName; (void)usStackDepth; (void)pvParameters; (void)uxPriority; (void)pxCreatedTask;
    return pdPASS;
}

void vTaskStartScheduler(void) {}
void vTaskDelay(TickType_t xTicksToDelay) { (void)xTicksToDelay; }

// Test utilities
void reset_test_state(void) {
    hal_uart_transmit_count = 0;
    hal_uart_receive_it_count = 0;
    hal_gpio_toggle_pin_count = 0;
    memset(last_transmitted_data, 0, sizeof(last_transmitted_data));
    last_transmitted_length = 0;
    
    // Reset global variables
    memset(&uart_rb, 0, sizeof(uart_rb));
    Serial_flag = 0;
    rx_data = 0;
}

void print_test_result(const char *test_name, int passed) {
    printf("[%s] %s\n", passed ? "PASS" : "FAIL", test_name);
}

#define TEST_ASSERT(condition, test_name) \
    do { \
        if (condition) { \
            print_test_result(test_name, 1); \
        } else { \
            print_test_result(test_name, 0); \
            return 0; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s...\n", #test_func); \
        if (test_func()) { \
            total_tests++; \
            passed_tests++; \
        } else { \
            total_tests++; \
        } \
    } while(0)

// Include the functions from app.c that we need to test
extern ring_buf_t uart_rb;
extern uint8_t rx_data;
extern const char target_str[];
extern uint8_t Serial_flag;

static inline void ring_write(ring_buf_t *rb, uint8_t data) {
    uint16_t next = (rb->head + 1) % 128;
    if (next != rb->tail) {
        rb->buf[rb->head] = data;
        rb->head = next;
    }
}

static inline int ring_read(ring_buf_t *rb, uint8_t *data) {
    if (rb->head == rb->tail)
        return 0;
    *data = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % 128;
    return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        ring_write(&uart_rb, rx_data);
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    }
}

// Integration Test 1: Complete UART communication flow
int test_complete_uart_flow(void) {
    reset_test_state();
    
    // Simulate receiving a valid frame via UART
    uint8_t valid_frame[] = {0xAA, 0x55, 'B', '8', '8', '8', '8', '8', 0xCC};
    
    // Initialize UART receive
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    TEST_ASSERT(hal_uart_receive_it_count == 1, "UART receive initialized");
    
    // Simulate each byte being received via interrupt
    for (int i = 0; i < sizeof(valid_frame); i++) {
        rx_data = valid_frame[i];
        HAL_UART_RxCpltCallback(&huart1);
    }
    
    // Process the data through UART task logic
    state_t state = WAIT_HEAD1;
    uint8_t data;
    uint8_t data_buf[32] = {0};
    uint8_t data_count = 0;
    int frames_processed = 0;
    
    while (ring_read(&uart_rb, &data)) {
        switch (state) {
            case WAIT_HEAD1:
                if (data == 0xAA) state = WAIT_HEAD2;
                break;
            case WAIT_HEAD2:
                if (data == 0x55) {
                    data_count = 0;
                    state = WAIT_DATA;
                } else {
                    state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
                }
                break;
            case WAIT_DATA:
                if (data == 0xCC) {
                    data_buf[data_count] = '\0';
                    HAL_UART_Transmit(&huart1, data_buf, data_count, 100);
                    state = WAIT_HEAD1;
                    Serial_flag = 1;
                    frames_processed++;
                    
                    if (strstr((char *)data_buf, target_str)) {
                        const char reply[] = "Received target!\r\n";
                        HAL_UART_Transmit(&huart1, (uint8_t *)reply, strlen(reply), 100);
                        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
                    }
                } else {
                    if (data_count < 31) {
                        data_buf[data_count++] = data;
                    }
                }
                break;
        }
    }
    
    TEST_ASSERT(frames_processed == 1, "One frame processed");
    TEST_ASSERT(strcmp((char *)data_buf, "B88888") == 0, "Correct data extracted");
    TEST_ASSERT(Serial_flag == 1, "Serial flag set");
    TEST_ASSERT(hal_uart_transmit_count >= 2, "Data and reply transmitted"); // Data echo + reply
    TEST_ASSERT(hal_gpio_toggle_pin_count == 1, "GPIO toggled for target");
    
    return 1;
}

// Integration Test 2: High-frequency data reception
int test_high_frequency_data(void) {
    reset_test_state();
    
    // Simulate rapid data reception (multiple frames in quick succession)
    uint8_t frames[][9] = {
        {0xAA, 0x55, 'B', '8', '8', '8', '8', '8', 0xCC},  // Target
        {0xAA, 0x55, 'A', '1', '2', '3', '4', '5', 0xCC},  // Non-target
        {0xAA, 0x55, 'C', '6', '7', '8', '9', '0', 0xCC},  // Non-target
        {0xAA, 0x55, 'B', '8', '8', '8', '8', '8', 0xCC}   // Target again
    };
    
    int num_frames = sizeof(frames) / sizeof(frames[0]);
    
    // Fill ring buffer with all frames
    for (int i = 0; i < num_frames; i++) {
        for (int j = 0; j < 9; j++) {
            ring_write(&uart_rb, frames[i][j]);
        }
    }
    
    // Process all frames
    state_t state = WAIT_HEAD1;
    uint8_t data;
    uint8_t data_buf[32] = {0};
    uint8_t data_count = 0;
    int frames_processed = 0;
    int target_frames_found = 0;
    
    while (ring_read(&uart_rb, &data)) {
        switch (state) {
            case WAIT_HEAD1:
                if (data == 0xAA) state = WAIT_HEAD2;
                break;
            case WAIT_HEAD2:
                if (data == 0x55) {
                    data_count = 0;
                    state = WAIT_DATA;
                } else {
                    state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
                }
                break;
            case WAIT_DATA:
                if (data == 0xCC) {
                    data_buf[data_count] = '\0';
                    HAL_UART_Transmit(&huart1, data_buf, data_count, 100);
                    state = WAIT_HEAD1;
                    Serial_flag = 1;
                    frames_processed++;
                    
                    if (strstr((char *)data_buf, target_str)) {
                        const char reply[] = "Received target!\r\n";
                        HAL_UART_Transmit(&huart1, (uint8_t *)reply, strlen(reply), 100);
                        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
                        target_frames_found++;
                    }
                    
                    // Reset for next frame
                    memset(data_buf, 0, sizeof(data_buf));
                    data_count = 0;
                } else {
                    if (data_count < 31) {
                        data_buf[data_count++] = data;
                    }
                }
                break;
        }
    }
    
    TEST_ASSERT(frames_processed == num_frames, "All frames processed");
    TEST_ASSERT(target_frames_found == 2, "Target frames correctly identified");
    TEST_ASSERT(hal_gpio_toggle_pin_count == 2, "GPIO toggled for target frames");
    TEST_ASSERT(hal_uart_transmit_count >= num_frames + 2, "All data transmitted"); // Frame echoes + replies
    
    return 1;
}

// Integration Test 3: Mixed valid and invalid data
int test_mixed_valid_invalid_data(void) {
    reset_test_state();
    
    // Create a mixed sequence: valid frame, invalid data, valid frame
    uint8_t mixed_data[] = {
        // Valid frame 1
        0xAA, 0x55, 'A', '1', '2', '3', '4', '5', 0xCC,
        // Invalid data (missing headers)
        0x01, 0x02, 0x03,
        // Partial frame (incomplete)
        0xAA, 0x55, 'X', 'Y',
        // Valid frame 2 (target)
        0xAA, 0x55, 'B', '8', '8', '8', '8', '8', 0xCC,
        // Invalid frame (wrong tail)
        0xAA, 0x55, 'Z', '9', '9', '9', '9', '9', 0xDD
    };
    
    // Add all data to ring buffer
    for (int i = 0; i < sizeof(mixed_data); i++) {
        ring_write(&uart_rb, mixed_data[i]);
    }
    
    // Process the mixed data
    state_t state = WAIT_HEAD1;
    uint8_t data;
    uint8_t data_buf[32] = {0};
    uint8_t data_count = 0;
    int valid_frames_processed = 0;
    int target_frames_found = 0;
    
    while (ring_read(&uart_rb, &data)) {
        switch (state) {
            case WAIT_HEAD1:
                if (data == 0xAA) state = WAIT_HEAD2;
                break;
            case WAIT_HEAD2:
                if (data == 0x55) {
                    data_count = 0;
                    state = WAIT_DATA;
                } else {
                    state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
                }
                break;
            case WAIT_DATA:
                if (data == 0xCC) {
                    data_buf[data_count] = '\0';
                    HAL_UART_Transmit(&huart1, data_buf, data_count, 100);
                    state = WAIT_HEAD1;
                    Serial_flag = 1;
                    valid_frames_processed++;
                    
                    if (strstr((char *)data_buf, target_str)) {
                        const char reply[] = "Received target!\r\n";
                        HAL_UART_Transmit(&huart1, (uint8_t *)reply, strlen(reply), 100);
                        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
                        target_frames_found++;
                    }
                    
                    memset(data_buf, 0, sizeof(data_buf));
                    data_count = 0;
                } else {
                    if (data_count < 31) {
                        data_buf[data_count++] = data;
                    }
                }
                break;
        }
    }
    
    TEST_ASSERT(valid_frames_processed == 2, "Only valid frames processed");
    TEST_ASSERT(target_frames_found == 1, "Only one target frame found");
    TEST_ASSERT(hal_gpio_toggle_pin_count == 1, "GPIO toggled only for target");
    
    return 1;
}

// Integration Test 4: Buffer boundary conditions
int test_buffer_boundary_conditions(void) {
    reset_test_state();
    
    // Test ring buffer wraparound during frame processing
    uint8_t large_frame[] = {0xAA, 0x55};
    uint8_t frame_data[120]; // Large data payload
    
    // Fill frame data
    for (int i = 0; i < 120; i++) {
        frame_data[i] = 'A' + (i % 26);
    }
    
    // Start filling ring buffer near the end to force wraparound
    uart_rb.head = 120;
    uart_rb.tail = 120;
    
    // Write frame header
    ring_write(&uart_rb, large_frame[0]);
    ring_write(&uart_rb, large_frame[1]);
    
    // Write frame data (this will cause wraparound)
    for (int i = 0; i < 10; i++) { // Write 10 bytes for testing
        ring_write(&uart_rb, frame_data[i]);
    }
    
    // Write frame tail
    ring_write(&uart_rb, 0xCC);
    
    // Process the frame
    state_t state = WAIT_HEAD1;
    uint8_t data;
    uint8_t data_buf[32] = {0};
    uint8_t data_count = 0;
    int frames_processed = 0;
    
    while (ring_read(&uart_rb, &data)) {
        switch (state) {
            case WAIT_HEAD1:
                if (data == 0xAA) state = WAIT_HEAD2;
                break;
            case WAIT_HEAD2:
                if (data == 0x55) {
                    data_count = 0;
                    state = WAIT_DATA;
                } else {
                    state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
                }
                break;
            case WAIT_DATA:
                if (data == 0xCC) {
                    data_buf[data_count] = '\0';
                    state = WAIT_HEAD1;
                    frames_processed++;
                    
                    // Verify data integrity
                    TEST_ASSERT(data_count == 10, "Correct number of data bytes processed");
                    for (int i = 0; i < 10; i++) {
                        TEST_ASSERT(data_buf[i] == frame_data[i], "Data integrity preserved across wraparound");
                    }
                } else {
                    if (data_count < 31) {
                        data_buf[data_count++] = data;
                    }
                }
                break;
        }
    }
    
    TEST_ASSERT(frames_processed == 1, "Frame processed correctly across buffer boundary");
    
    return 1;
}

// Integration Test 5: System resource management
int test_system_resource_management(void) {
    reset_test_state();
    
    // Test that the system properly manages resources under load
    const int num_iterations = 100;
    int total_gpio_toggles = 0;
    
    for (int iter = 0; iter < num_iterations; iter++) {
        // Simulate LED task operation
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        total_gpio_toggles++;
        
        // Simulate occasional UART reception
        if (iter % 10 == 0) {
            uint8_t test_byte = 0x55;
            rx_data = test_byte;
            HAL_UART_RxCpltCallback(&huart1);
        }
    }
    
    TEST_ASSERT(hal_gpio_toggle_pin_count == total_gpio_toggles, "GPIO operations tracked correctly");
    TEST_ASSERT(hal_uart_receive_it_count == 10, "UART interrupts properly handled");
    
    return 1;
}

// Integration Test 6: Performance under stress
int test_performance_stress(void) {
    reset_test_state();
    
    clock_t start_time = clock();
    
    // Process a large number of frames quickly
    const int num_frames = 1000;
    
    for (int i = 0; i < num_frames; i++) {
        uint8_t frame[] = {0xAA, 0x55, 'T', 'E', 'S', 'T', '0' + (i % 10), '0' + (i / 10) % 10, 0xCC};
        
        // Add frame to ring buffer
        for (int j = 0; j < 9; j++) {
            ring_write(&uart_rb, frame[j]);
        }
        
        // Process immediately (simulating real-time processing)
        state_t state = WAIT_HEAD1;
        uint8_t data;
        uint8_t data_buf[32] = {0};
        uint8_t data_count = 0;
        int frames_in_batch = 0;
        
        while (ring_read(&uart_rb, &data) && frames_in_batch < 1) { // Process only one frame
            switch (state) {
                case WAIT_HEAD1:
                    if (data == 0xAA) state = WAIT_HEAD2;
                    break;
                case WAIT_HEAD2:
                    if (data == 0x55) {
                        data_count = 0;
                        state = WAIT_DATA;
                    } else {
                        state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
                    }
                    break;
                case WAIT_DATA:
                    if (data == 0xCC) {
                        data_buf[data_count] = '\0';
                        HAL_UART_Transmit(&huart1, data_buf, data_count, 100);
                        state = WAIT_HEAD1;
                        frames_in_batch++;
                    } else {
                        if (data_count < 31) {
                            data_buf[data_count++] = data;
                        }
                    }
                    break;
            }
        }
    }
    
    clock_t end_time = clock();
    double processing_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    TEST_ASSERT(hal_uart_transmit_count == num_frames, "All frames processed under stress");
    TEST_ASSERT(processing_time < 1.0, "Processing time acceptable under stress"); // Should complete within 1 second
    
    printf("    Processed %d frames in %.4f seconds\n", num_frames, processing_time);
    
    return 1;
}

// Integration Test 7: Error recovery
int test_error_recovery(void) {
    reset_test_state();
    
    // Test system recovery from various error conditions
    
    // Error 1: Corrupted frame headers
    uint8_t corrupted_headers[] = {0x55, 0xAA, 0xAA, 0x55}; // Swapped headers
    for (int i = 0; i < sizeof(corrupted_headers); i++) {
        ring_write(&uart_rb, corrupted_headers[i]);
    }
    
    // Process and verify recovery
    state_t state = WAIT_HEAD1;
    uint8_t data;
    int valid_frames_found = 0;
    
    while (ring_read(&uart_rb, &data)) {
        switch (state) {
            case WAIT_HEAD1:
                if (data == 0xAA) state = WAIT_HEAD2;
                break;
            case WAIT_HEAD2:
                if (data == 0x55) {
                    state = WAIT_DATA;
                    valid_frames_found++;
                } else {
                    state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
                }
                break;
            case WAIT_DATA:
                // Skip data processing for this test
                break;
        }
    }
    
    TEST_ASSERT(valid_frames_found == 1, "System recovers from corrupted headers");
    
    // Error 2: Frame without tail (should be ignored)
    reset_test_state();
    uint8_t incomplete_frame[] = {0xAA, 0x55, 'I', 'N', 'C', 'O', 'M', 'P'};
    for (int i = 0; i < sizeof(incomplete_frame); i++) {
        ring_write(&uart_rb, incomplete_frame[i]);
    }
    
    // Process incomplete frame
    state = WAIT_HEAD1;
    while (ring_read(&uart_rb, &data)) {
        switch (state) {
            case WAIT_HEAD1:
                if (data == 0xAA) state = WAIT_HEAD2;
                break;
            case WAIT_HEAD2:
                if (data == 0x55) {
                    state = WAIT_DATA;
                } else {
                    state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
                }
                break;
            case WAIT_DATA:
                // Should stay in WAIT_DATA since no tail received
                break;
        }
    }
    
    TEST_ASSERT(state == WAIT_DATA, "Incomplete frame leaves system in waiting state");
    TEST_ASSERT(Serial_flag == 0, "Incomplete frame doesn't trigger actions");
    
    return 1;
}

// Main test runner
int main(void) {
    printf("=== Smart Garage Application Integration Tests ===\n\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run all integration tests
    RUN_TEST(test_complete_uart_flow);
    RUN_TEST(test_high_frequency_data);
    RUN_TEST(test_mixed_valid_invalid_data);
    RUN_TEST(test_buffer_boundary_conditions);
    RUN_TEST(test_system_resource_management);
    RUN_TEST(test_performance_stress);
    RUN_TEST(test_error_recovery);
    
    // Print summary
    printf("\n=== Integration Test Summary ===\n");
    printf("Total Tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    printf("Success Rate: %.1f%%\n", total_tests > 0 ? (float)passed_tests / total_tests * 100 : 0);
    
    if (passed_tests == total_tests) {
        printf("\n🎉 All integration tests passed! System components work together correctly.\n");
    } else {
        printf("\n❌ Some integration tests failed. Check component interactions.\n");
    }
    
    return (passed_tests == total_tests) ? 0 : 1;
}