# Directories
LIBS = libUVA libiaea libparticleDmlc
DEMO = particleDmlc++Demo
LIB_DIR = x86_64-linux

# Rule to compile all libraries and demo
all: compile_libs compile_demo

# Step 1: Compile each library in its subdirectory
compile_libs:
	@for lib in $(LIBS); do \
		echo "Compiling $$lib..."; \
		$(MAKE) -C $$lib; \
	done

# Step 2: Compile the demo
compile_demo:
	@echo "Compiling particleDmlc++Demo..."
	$(MAKE) -C $(DEMO)

# Clean all directories
clean:
	@for lib in $(LIBS); do \
		echo "Cleaning $$lib..."; \
		$(MAKE) -C $$lib clean; \
	done
	@echo "Cleaning particleDmlc++Demo..."
	$(MAKE) -C $(DEMO) clean
	@echo "Cleaning lib directory $(LIB_DIR)..."
	rm -rf $(LIB_DIR)

