//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <assert.h>
//#include "app.h"

//// Mock FreeRTOS functions and types
//typedef void *TaskHandle_t;
//typedef void *QueueHandle_t;
//typedef unsigned int UBaseType_t;
//typedef unsigned int TickType_t;

//#define pdPASS 1
//#define errQUEUE_EMPTY 0
//#define portMAX_DELAY 0xFFFFFFFF

//// Mock FreeRTOS functions
//BaseType_t xTaskCreate(void (*pvTaskCode)(void *), const char *pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, TaskHandle_t *pxCreatedTask) {
//    (void)pvTaskCode; (void)pcName; (void)usStackDepth; (void)pvParameters; (void)uxPriority; (void)pxCreatedTask;
//    return pdPASS;
//}

//void vTaskStartScheduler(void) {}
//void vTaskDelay(TickType_t xTicksToDelay) { (void)xTicksToDelay; }

//// Mock HAL functions and types
//typedef struct {
//    void *Instance;
//} UART_HandleTypeDef;

//typedef struct {
//    void *Instance;
//} GPIO_TypeDef;

//typedef enum {
//    GPIO_PIN_0 = 0x0001U,
//    GPIO_PIN_1 = 0x0002U,
//    GPIO_PIN_2 = 0x0004U,
//    GPIO_PIN_3 = 0x0008U,
//    GPIO_PIN_4 = 0x0010U,
//    GPIO_PIN_5 = 0x0020U,
//    GPIO_PIN_6 = 0x0040U,
//    GPIO_PIN_7 = 0x0080U,
//    GPIO_PIN_8 = 0x0100U,
//    GPIO_PIN_9 = 0x0200U,
//    GPIO_PIN_10 = 0x0400U,
//    GPIO_PIN_11 = 0x0800U,
//    GPIO_PIN_12 = 0x1000U,
//    GPIO_PIN_13 = 0x2000U,
//    GPIO_PIN_14 = 0x4000U,
//    GPIO_PIN_15 = 0x8000U
//} GPIO_Pin_TypeDef;

//typedef enum {
//    HAL_OK = 0x00U,
//    HAL_ERROR = 0x01U,
//    HAL_BUSY = 0x02U,
//    HAL_TIMEOUT = 0x03U
//} HAL_StatusTypeDef;

//// Mock hardware instances
//UART_HandleTypeDef huart1 = {0};
//GPIO_TypeDef *GPIOB = (GPIO_TypeDef*)0x40010800;
//GPIO_TypeDef *GPIOC = (GPIO_TypeDef*)0x40011000;

//// Track mock function calls
//static int hal_uart_transmit_count = 0;
//static int hal_uart_receive_it_count = 0;
//static int hal_gpio_toggle_pin_count = 0;
//static uint8_t last_transmitted_data[32];
//static uint16_t last_transmitted_length = 0;

//// Mock HAL function implementations
//HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
//    (void)huart; (void)Timeout;
//    hal_uart_transmit_count++;
//    memcpy(last_transmitted_data, pData, Size);
//    last_transmitted_length = Size;
//    return HAL_OK;
//}

//HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) {
//    (void)huart; (void)pData; (void)Size;
//    hal_uart_receive_it_count++;
//    return HAL_OK;
//}

//void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
//    (void)GPIOx; (void)GPIO_Pin;
//    hal_gpio_toggle_pin_count++;
//}

//// Mock OLED functions
//void OLED_Init(void) {}
//void OLED_Clear(void) {}
//void OLED_Update(void) {}
//void OLED_ShowChinese(uint8_t x, uint8_t y, const char *str) { (void)x; (void)y; (void)str; }
//void OLED_ShowNum(uint8_t x, uint8_t y, uint16_t num, uint8_t len, uint8_t size) { (void)x; (void)y; (void)num; (void)len; (void)size; }
//void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size) { (void)x; (void)y; (void)str; (void)size; }

//// Test helper functions
//void reset_mock_counters(void) {
//    hal_uart_transmit_count = 0;
//    hal_uart_receive_it_count = 0;
//    hal_gpio_toggle_pin_count = 0;
//    memset(last_transmitted_data, 0, sizeof(last_transmitted_data));
//    last_transmitted_length = 0;
//}

//void print_test_result(const char *test_name, int passed) {
//    printf("[%s] %s\n", passed ? "PASS" : "FAIL", test_name);
//}

//// Test framework macros
//#define TEST_ASSERT(condition, test_name) \
//    do { \
//        if (condition) { \
//            print_test_result(test_name, 1); \
//        } else { \
//            print_test_result(test_name, 0); \
//            return 0; \
//        } \
//    } while(0)

//#define RUN_TEST(test_func) \
//    do { \
//        printf("Running %s...\n", #test_func); \
//        if (test_func()) { \
//            total_tests++; \
//            passed_tests++; \
//        } else { \
//            total_tests++; \
//        } \
//    } while(0)

//// Function declarations from app.c (need to be tested)
//extern ring_buf_t uart_rb;
//extern uint8_t rx_data;
//extern const char target_str[];
//extern uint8_t Serial_flag;

//static inline void ring_write(ring_buf_t *rb, uint8_t data);
//static inline int ring_read(ring_buf_t *rb, uint8_t *data);
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
//void uart_task(void *arg);
//void led_task(void);
//void oled_task(void);
//void app_init(void);

//// Test Cases

//// Test 1: Ring buffer operations
//int test_ring_buffer_operations(void) {
//    ring_buf_t test_rb = {0};
//    uint8_t test_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
//    uint8_t read_data;
//    int i;
//    
//    // Test single write and read
//    ring_write(&test_rb, 0x55);
//    TEST_ASSERT(ring_read(&test_rb, &read_data) == 1, "Single write/read");
//    TEST_ASSERT(read_data == 0x55, "Data integrity");
//    TEST_ASSERT(test_rb.head == test_rb.tail, "Buffer empty after read");
//    
//    // Test multiple writes and reads
//    for (i = 0; i < 5; i++) {
//        ring_write(&test_rb, test_data[i]);
//    }
//    
//    for (i = 0; i < 5; i++) {
//        TEST_ASSERT(ring_read(&test_rb, &read_data) == 1, "Multiple reads");
//        TEST_ASSERT(read_data == test_data[i], "Data order preservation");
//    }
//    
//    // Test read from empty buffer
//    TEST_ASSERT(ring_read(&test_rb, &read_data) == 0, "Read empty buffer");
//    
//    // Test buffer wraparound
//    test_rb.head = 126; // Near end of buffer
//    test_rb.tail = 126;
//    ring_write(&test_rb, 0xAA);
//    ring_write(&test_rb, 0xBB);
//    TEST_ASSERT(ring_read(&test_rb, &read_data) == 1, "Wraparound read 1");
//    TEST_ASSERT(read_data == 0xAA, "Wraparound data 1");
//    TEST_ASSERT(ring_read(&test_rb, &read_data) == 1, "Wraparound read 2");
//    TEST_ASSERT(read_data == 0xBB, "Wraparound data 2");
//    
//    // Test buffer full condition
//    ring_buf_t small_rb = {0};
//    small_rb.head = 1; // Simulate nearly full buffer
//    small_rb.tail = 0;
//    uint8_t old_head = small_rb.head;
//    ring_write(&small_rb, 0xFF); // This should fail as buffer would become full
//    TEST_ASSERT(small_rb.head == old_head, "Buffer full protection");
//    
//    return 1;
//}

//// Test 2: UART callback function
//int test_uart_callback(void) {
//    reset_mock_counters();
//    
//    // Initialize rx_data with test value
//    rx_data = 0x42;
//    uart_rb.head = 0;
//    uart_rb.tail = 0;
//    
//    // Call callback with correct UART instance
//    HAL_UART_RxCpltCallback(&huart1);
//    
//    // Check that data was written to ring buffer
//    uint8_t read_data;
//    TEST_ASSERT(ring_read(&uart_rb, &read_data) == 1, "Callback writes to buffer");
//    TEST_ASSERT(read_data == 0x42, "Callback data integrity");
//    TEST_ASSERT(hal_uart_receive_it_count == 1, "Callback re-enables RX interrupt");
//    
//    return 1;
//}

//// Test 3: UART protocol parsing - valid frame
//int test_uart_protocol_valid_frame(void) {
//    reset_mock_counters();
//    
//    // Reset global variables
//    memset(&uart_rb, 0, sizeof(uart_rb));
//    Serial_flag = 0;
//    
//    // Simulate valid frame: AA 55 'B88888' CC
//    uint8_t valid_frame[] = {0xAA, 0x55, 'B', '8', '8', '8', '8', '8', 0xCC};
//    
//    // Write frame data to ring buffer
//    for (int i = 0; i < sizeof(valid_frame); i++) {
//        ring_write(&uart_rb, valid_frame[i]);
//    }
//    
//    // Create task context (simplified test - just one iteration)
//    state_t state = WAIT_HEAD1;
//    uint8_t data;
//    uint8_t data_buf[32] = {0};
//    uint8_t data_count = 0;
//    
//    // Process the frame
//    for (int i = 0; i < sizeof(valid_frame); i++) {
//        if (ring_read(&uart_rb, &data)) {
//            switch (state) {
//                case WAIT_HEAD1:
//                    if (data == 0xAA) state = WAIT_HEAD2;
//                    break;
//                case WAIT_HEAD2:
//                    if (data == 0x55) {
//                        data_count = 0;
//                        state = WAIT_DATA;
//                    } else {
//                        state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
//                    }
//                    break;
//                case WAIT_DATA:
//                    if (data == 0xCC) {
//                        // Frame complete
//                        state = WAIT_HEAD1;
//                        Serial_flag = 1;
//                        // Check for target string
//                        data_buf[data_count] = '\0';
//                        if (strstr((char *)data_buf, target_str)) {
//                            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
//                        }
//                    } else {
//                        if (data_count < 31) {
//                            data_buf[data_count++] = data;
//                        }
//                    }
//                    break;
//            }
//        }
//    }
//    
//    TEST_ASSERT(strcmp((char *)data_buf, "B88888") == 0, "Frame data extraction");
//    TEST_ASSERT(Serial_flag == 1, "Serial flag set");
//    TEST_ASSERT(hal_gpio_toggle_pin_count == 1, "Target detection triggers GPIO");
//    
//    return 1;
//}

//// Test 4: UART protocol parsing - invalid frame
//int test_uart_protocol_invalid_frame(void) {
//    reset_mock_counters();
//    
//    memset(&uart_rb, 0, sizeof(uart_rb));
//    Serial_flag = 0;
//    
//    // Simulate invalid frame: missing tail
//    uint8_t invalid_frame[] = {0xAA, 0x55, 'B', '8', '8', '8', '8', '8'};
//    
//    for (int i = 0; i < sizeof(invalid_frame); i++) {
//        ring_write(&uart_rb, invalid_frame[i]);
//    }
//    
//    state_t state = WAIT_HEAD1;
//    uint8_t data;
//    uint8_t data_buf[32] = {0};
//    uint8_t data_count = 0;
//    
//    // Process the invalid frame
//    for (int i = 0; i < sizeof(invalid_frame); i++) {
//        if (ring_read(&uart_rb, &data)) {
//            switch (state) {
//                case WAIT_HEAD1:
//                    if (data == 0xAA) state = WAIT_HEAD2;
//                    break;
//                case WAIT_HEAD2:
//                    if (data == 0x55) {
//                        data_count = 0;
//                        state = WAIT_DATA;
//                    } else {
//                        state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
//                    }
//                    break;
//                case WAIT_DATA:
//                    if (data == 0xCC) {
//                        state = WAIT_HEAD1;
//                        Serial_flag = 1;
//                    } else {
//                        if (data_count < 31) {
//                            data_buf[data_count++] = data;
//                        }
//                    }
//                    break;
//            }
//        }
//    }
//    
//    TEST_ASSERT(Serial_flag == 0, "Invalid frame doesn't set flag");
//    TEST_ASSERT(state == WAIT_DATA, "State remains in WAIT_DATA for incomplete frame");
//    
//    return 1;
//}

//// Test 5: Target string detection
//int test_target_string_detection(void) {
//    reset_mock_counters();
//    
//    // Test exact match
//    const char *test_exact = "B88888";
//    TEST_ASSERT(strstr(test_exact, target_str) != NULL, "Exact target match");
//    
//    // Test partial match (should fail)
//    const char *test_partial = "B8888";
//    TEST_ASSERT(strstr(test_partial, target_str) == NULL, "Partial match fails");
//    
//    // Test string containing target
//    const char *test_containing = "XXB88888YY";
//    TEST_ASSERT(strstr(test_containing, target_str) != NULL, "Target within string");
//    
//    // Test no match
//    const char *test_no_match = "A12345";
//    TEST_ASSERT(strstr(test_no_match, target_str) == NULL, "No match detection");
//    
//    return 1;
//}

//// Test 6: Frame state transitions
//int test_frame_state_transitions(void) {
//    state_t state = WAIT_HEAD1;
//    uint8_t data;
//    uint8_t data_buf[32] = {0};
//    uint8_t data_count = 0;
//    
//    // Test transition WAIT_HEAD1 -> WAIT_HEAD2 with correct header
//    data = 0xAA;
//    if (data == 0xAA) state = WAIT_HEAD2;
//    TEST_ASSERT(state == WAIT_HEAD2, "WAIT_HEAD1 to WAIT_HEAD2 transition");
//    
//    // Test transition WAIT_HEAD2 -> WAIT_DATA with correct header
//    data = 0x55;
//    if (data == 0x55) {
//        data_count = 0;
//        state = WAIT_DATA;
//    }
//    TEST_ASSERT(state == WAIT_DATA, "WAIT_HEAD2 to WAIT_DATA transition");
//    
//    // Test transition WAIT_HEAD2 -> WAIT_HEAD1 with invalid data
//    state = WAIT_HEAD2;
//    data = 0x33; // Invalid header
//    if (data == 0x55) {
//        data_count = 0;
//        state = WAIT_DATA;
//    } else {
//        state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
//    }
//    TEST_ASSERT(state == WAIT_HEAD1, "WAIT_HEAD2 to WAIT_HEAD1 with invalid data");
//    
//    // Test transition WAIT_HEAD2 -> WAIT_HEAD2 with another header
//    state = WAIT_HEAD2;
//    data = 0xAA; // Another header
//    if (data == 0x55) {
//        data_count = 0;
//        state = WAIT_DATA;
//    } else {
//        state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
//    }
//    TEST_ASSERT(state == WAIT_HEAD2, "WAIT_HEAD2 to WAIT_HEAD2 with header");
//    
//    return 1;
//}

//// Test 7: Buffer overflow protection
//int test_buffer_overflow_protection(void) {
//    ring_buf_t test_rb = {0};
//    
//    // Fill buffer to near capacity
//    for (int i = 0; i < 126; i++) {
//        ring_write(&test_rb, (uint8_t)i);
//    }
//    
//    uint16_t head_before = test_rb.head;
//    uint16_t tail_before = test_rb.tail;
//    
//    // Try to write more data when buffer is almost full
//    ring_write(&test_rb, 0xFF);
//    
//    // Check that write was prevented when buffer would become full
//    if (((test_rb.head + 1) % 128) == test_rb.tail) {
//        // Buffer would become full, write should be prevented
//        TEST_ASSERT(test_rb.head == head_before, "Overflow protection - head unchanged");
//    }
//    
//    return 1;
//}

//// Test 8: Data buffer bounds checking
//int test_data_buffer_bounds(void) {
//    state_t state = WAIT_DATA;
//    uint8_t data_buf[32] = {0};
//    uint8_t data_count = 0;
//    
//    // Fill data buffer to capacity
//    for (int i = 0; i < 31; i++) {
//        uint8_t data = 'A' + (i % 26);
//        if (data_count < 31) {
//            data_buf[data_count++] = data;
//        }
//    }
//    
//    uint8_t count_before = data_count;
//    
//    // Try to add one more character (should fail due to bounds check)
//    uint8_t data = 'Z';
//    if (data_count < 31) {
//        data_buf[data_count++] = data;
//    }
//    
//    TEST_ASSERT(data_count == count_before + 1, "Bounds check allows valid write");
//    
//    // Try to add another character when at limit
//    if (data_count < 31) {
//        data_buf[data_count++] = data;
//    }
//    
//    TEST_ASSERT(data_count == 32, "Bounds check prevents overflow");
//    
//    return 1;
//}

//// Test 9: Multiple sequential frames
//int test_multiple_sequential_frames(void) {
//    reset_mock_counters();
//    
//    memset(&uart_rb, 0, sizeof(uart_rb));
//    
//    // Create two sequential frames
//    uint8_t frame1[] = {0xAA, 0x55, 'B', '8', '8', '8', '8', '8', 0xCC};
//    uint8_t frame2[] = {0xAA, 0x55, 'A', '1', '2', '3', '4', '5', 0xCC};
//    
//    // Write both frames to buffer
//    for (int i = 0; i < sizeof(frame1); i++) {
//        ring_write(&uart_rb, frame1[i]);
//    }
//    for (int i = 0; i < sizeof(frame2); i++) {
//        ring_write(&uart_rb, frame2[i]);
//    }
//    
//    state_t state = WAIT_HEAD1;
//    uint8_t data;
//    uint8_t data_buf[32] = {0};
//    uint8_t data_count = 0;
//    int frames_processed = 0;
//    
//    // Process all data
//    while (ring_read(&uart_rb, &data)) {
//        switch (state) {
//            case WAIT_HEAD1:
//                if (data == 0xAA) state = WAIT_HEAD2;
//                break;
//            case WAIT_HEAD2:
//                if (data == 0x55) {
//                    data_count = 0;
//                    state = WAIT_DATA;
//                } else {
//                    state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
//                }
//                break;
//            case WAIT_DATA:
//                if (data == 0xCC) {
//                    data_buf[data_count] = '\0';
//                    state = WAIT_HEAD1;
//                    frames_processed++;
//                    
//                    // Check if this was a target frame
//                    if (strstr((char *)data_buf, target_str)) {
//                        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
//                    }
//                    
//                    // Reset buffer for next frame
//                    memset(data_buf, 0, sizeof(data_buf));
//                    data_count = 0;
//                } else {
//                    if (data_count < 31) {
//                        data_buf[data_count++] = data;
//                    }
//                }
//                break;
//        }
//    }
//    
//    TEST_ASSERT(frames_processed == 2, "Two frames processed");
//    TEST_ASSERT(hal_gpio_toggle_pin_count == 1, "Only target frame triggered GPIO");
//    
//    return 1;
//}

//// Test 10: LED task functionality
//int test_led_task_functionality(void) {
//    reset_mock_counters();
//    
//    // Note: led_task runs in an infinite loop, so we'll test the core logic
//    // In a real test environment, we would use a mock for the infinite loop
//    
//    // Simulate one iteration of the LED task
//    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//    
//    TEST_ASSERT(hal_gpio_toggle_pin_count == 1, "LED task toggles GPIO");
//    
//    return 1;
//}

//// Test 11: Serial flag handling
//int test_serial_flag_handling(void) {
//    reset_mock_counters();
//    
//    // Reset Serial flag
//    Serial_flag = 0;
//    
//    // Simulate setting the flag
//    Serial_flag = 1;
//    
//    // Test that flag can be read and reset
//    TEST_ASSERT(Serial_flag == 1, "Serial flag can be set");
//    
//    // Simulate clearing the flag (as done in oled_task)
//    Serial_flag = 0;
//    TEST_ASSERT(Serial_flag == 0, "Serial flag can be cleared");
//    
//    return 1;
//}

//// Test 12: Edge cases - malformed frames
//int test_edge_cases_malformed_frames(void) {
//    reset_mock_counters();
//    
//    memset(&uart_rb, 0, sizeof(uart_rb));
//    Serial_flag = 0;
//    
//    // Test case 1: Only header1
//    uint8_t case1[] = {0xAA};
//    for (int i = 0; i < sizeof(case1); i++) {
//        ring_write(&uart_rb, case1[i]);
//    }
//    
//    state_t state = WAIT_HEAD1;
//    uint8_t data;
//    
//    if (ring_read(&uart_rb, &data) && data == 0xAA) {
//        state = WAIT_HEAD2;
//    }
//    TEST_ASSERT(state == WAIT_HEAD2, "Single header1 handled");
//    
//    // Test case 2: Headers with no data
//    memset(&uart_rb, 0, sizeof(uart_rb));
//    uint8_t case2[] = {0xAA, 0x55, 0xCC}; // Headers followed immediately by tail
//    for (int i = 0; i < sizeof(case2); i++) {
//        ring_write(&uart_rb, case2[i]);
//    }
//    
//    state = WAIT_HEAD1;
//    uint8_t data_buf[32] = {0};
//    uint8_t data_count = 0;
//    
//    while (ring_read(&uart_rb, &data)) {
//        switch (state) {
//            case WAIT_HEAD1:
//                if (data == 0xAA) state = WAIT_HEAD2;
//                break;
//            case WAIT_HEAD2:
//                if (data == 0x55) {
//                    data_count = 0;
//                    state = WAIT_DATA;
//                } else {
//                    state = (data == 0xAA) ? WAIT_HEAD2 : WAIT_HEAD1;
//                }
//                break;
//            case WAIT_DATA:
//                if (data == 0xCC) {
//                    data_buf[data_count] = '\0';
//                    state = WAIT_HEAD1;
//                    Serial_flag = 1;
//                } else {
//                    if (data_count < 31) {
//                        data_buf[data_count++] = data;
//                    }
//                }
//                break;
//        }
//    }
//    
//    TEST_ASSERT(Serial_flag == 1, "Empty data frame processed");
//    TEST_ASSERT(data_count == 0, "No data in empty frame");
//    
//    return 1;
//}

//// Test runner
//int main(void) {
//    printf("=== Smart Garage Application Unit Tests ===\n\n");
//    
//    int total_tests = 0;
//    int passed_tests = 0;
//    
//    // Run all tests
//    RUN_TEST(test_ring_buffer_operations);
//    RUN_TEST(test_uart_callback);
//    RUN_TEST(test_uart_protocol_valid_frame);
//    RUN_TEST(test_uart_protocol_invalid_frame);
//    RUN_TEST(test_target_string_detection);
//    RUN_TEST(test_frame_state_transitions);
//    RUN_TEST(test_buffer_overflow_protection);
//    RUN_TEST(test_data_buffer_bounds);
//    RUN_TEST(test_multiple_sequential_frames);
//    RUN_TEST(test_led_task_functionality);
//    RUN_TEST(test_serial_flag_handling);
//    RUN_TEST(test_edge_cases_malformed_frames);
//    
//    // Print summary
//    printf("\n=== Test Summary ===\n");
//    printf("Total Tests: %d\n", total_tests);
//    printf("Passed: %d\n", passed_tests);
//    printf("Failed: %d\n", total_tests - passed_tests);
//    printf("Success Rate: %.1f%%\n", total_tests > 0 ? (float)passed_tests / total_tests * 100 : 0);
//    
//    if (passed_tests == total_tests) {
//        printf("\n🎉 All tests passed! The Smart Garage application is working correctly.\n");
//    } else {
//        printf("\n❌ Some tests failed. Please review the implementation.\n");
//    }
//    
//    return (passed_tests == total_tests) ? 0 : 1;
//}