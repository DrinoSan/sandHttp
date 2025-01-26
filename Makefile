# Define the build directory and the executable name
BUILD_DIR := build
EXECUTABLE := httpServer

# Default target: build, copy, and run the executable
all: $(BUILD_DIR)/$(EXECUTABLE) copy

# Target to configure and build the executable inside the build directory
$(BUILD_DIR)/$(EXECUTABLE):
	mkdir -p $(BUILD_DIR)  # Ensure the build directory exists
	cd $(BUILD_DIR) && cmake ..  # Run cmake inside the build directory
	cd $(BUILD_DIR) && make  # Run make inside the build directory

# Target to copy the executable to the current directory
copy:
	cp $(BUILD_DIR)/$(EXECUTABLE) .  # Copy the executable to the current directory

# Target to run the executable
run: all
	./$(EXECUTABLE)  # Execute the copied executable

# Clean up build files
clean:
	rm -rf $(BUILD_DIR)  # Remove the build directory
	rm -f $(EXECUTABLE)  # Remove the executable from the current directory

