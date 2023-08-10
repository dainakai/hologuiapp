# Variables
CXX = g++
CXXFLAGS = -Wall -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets -I/usr/include/x86_64-linux-gnu/qt5/QtGui -I/usr/include/x86_64-linux-gnu/qt5/ -I/usr/include/opencv4 -I/usr/include/opencv4/opencv2/ -I/usr/loca/include -fPIC
LDFLAGS = -lQt5Widgets -lQt5Core -lQt5Gui -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lfftw3
MOC = moc
MOCFLAGS = 

# Source files
SRC = main.cpp
MOC_SRC = $(shell ls *.h)
MOC_OBJ = $(MOC_SRC:%.h=moc_%.o)
OBJ = $(SRC:.cpp=.o) $(MOC_OBJ)

# Output
TARGET = MyApp

# Rules
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

moc_%.o: %.h
	$(MOC) $(MOCFLAGS) $< -o moc_$*.cpp
	$(CXX) $(CXXFLAGS) -c moc_$*.cpp -o $@

clean:
	rm -f $(OBJ) $(TARGET)
	rm -f moc_*.cpp

.PHONY: all clean
