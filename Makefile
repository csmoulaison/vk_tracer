CFLAGS = -std=c17
LDFLAGS = -lSDL2main -lSDL2 -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
EXE = bin/vk_tracer
SRC = src/main.c

vk_tracer: $(SRC)
	gcc $(CFLAGS) -o $(EXE) $(SRC) $(LDFLAGS)

.PHONY: test clean

test: vk_tracer
	$(EXE)

clean:
	rm -f $(EXE)
