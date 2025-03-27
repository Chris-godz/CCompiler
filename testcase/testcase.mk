.PHONY : t1 t2

testdir :=  $(TOP_DIR)/testcase

t1:$(BUILD_DIR)/$(TARGET_EXEC)
	@valgrind --leak-check=full --track-origins=yes $(BUILD_DIR)/$(TARGET_EXEC) -koopa $(testdir)/v1.0/input.txt -o $(testdir)/v1.0/output.txt

t2:$(BUILD_DIR)/$(TARGET_EXEC)
	@valgrind --leak-check=full --track-origins=yes $(BUILD_DIR)/$(TARGET_EXEC) -riscv $(testdir)/v2.0/input.txt -o $(testdir)/v2.0/output.txt

t3:$(BUILD_DIR)/$(TARGET_EXEC)
	@$(BUILD_DIR)/$(TARGET_EXEC) -koopa $(testdir)/v3.0/input.txt -o $(testdir)/v3.0/output.txt

t4:$(BUILD_DIR)/$(TARGET_EXEC)
	@$(BUILD_DIR)/$(TARGET_EXEC) -koopa $(testdir)/v4.0/input.txt -o $(testdir)/v4.0/output.txt