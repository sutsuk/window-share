CCPP         = g++
MAIN_CPP     = main.cpp
MAIN_O       = main.o
DISPLAY_CPP  = display.cpp
DISPLAY_O    = display.o
SEND_CPP     = send.cpp
SEND_O       = send.o
SHARE_WINDOW = share-window

all:							$(SHARE_WINDOW)

$(MAIN_O):				$(MAIN_CPP)
									$(CCPP) -c $(MAIN_CPP) -lX11

$(DISPLAY_O):			$(DISPLAY_CPP)
									$(CCPP) -c $(DISPLAY_CPP) -lX11

$(SEND_O):				$(SEND_CPP)
									$(CCPP) -c $(SEND_CPP) -lX11

$(SHARE_WINDOW):	$(MAIN_O) $(DISPLAY_O) $(SEND_O)
									$(CCPP) $(MAIN_O) $(DISPLAY_O) $(SEND_O) -o $(SHARE_WINDOW) -lX11

run:							$(SHARE_WINDOW)
									./$(SHARE_WINDOW) 8888

clean:;						rm -rf $(MAIN_O) $(DISPLAY_O) $(SEND_O) $(SHARE_WINDOW)
