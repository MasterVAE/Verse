SOURCES_CORE = core/main.cpp core/core.cpp core/net_server.cpp core/game_server.cpp \
				core/data_manager.cpp net_request.cpp ../library/lib.cpp bot/neural_network.cpp bot/bot.cpp
				
TARGET_CORE = $(TARGET_DIR)/core.out

OBJECTS_CORE = $(addprefix $(OBJ_DIR)/, $(SOURCES_CORE:.cpp=.o))

$(TARGET_CORE): $(OBJECTS_CORE) | $(TARGET_DIR)
	@$(CC) $(CFLAGS) $^ -o $@
	@echo "LINKED CORE"

core_b: $(TARGET_CORE)

core: $(TARGET_CORE)
	@./$(TARGET_CORE)