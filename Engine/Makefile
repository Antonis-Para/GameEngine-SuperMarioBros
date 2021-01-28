CC=gcc
CFLAGS=
LDFLAGS=`pkg-config --libs allegro-5.0`

SOURCEDIR=Src/
OBJS=Animation.o Animator.o app.o Bitmap.o BoundingArea.o Color.o createNPCs.o Destruction.o GravityHandler.o GridLayer.o main.o opperations.o Sprite.o SystemClock.o TileLayer.o Utilities.o
INCLS=Animation.h Animator.h app.h Bitmap.h BoundingArea.h createNPCs.h Defines.h Destruction.h GravityHandler.h GridLayer.h Shapes.h Sprite.h States.h SystemClock.h TileLayer.h Utilities.h Typedefs.h
OUT=superMario

all: app_rule

clean:
        rm -rf *.o superMario

app_rule: $(OBJS) $(INCLS)
        $(CC) $(OBJS) -o $(OUT) $(CFLAGS) $(LDFLAGS)

%.o : %.c
        $(CC) -c $(CFLAGS) $(SOURCEDIR)$< -o $@