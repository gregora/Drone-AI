RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[1;36m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

echo "Compiling ..."

if g++ source/drone.cpp source/matrix.cpp source/network.cpp source/misc.cpp main.cpp -o main.out -lsfml-graphics -lsfml-window -lsfml-system -lpthread; then
	echo -e -n "${GREEN}"
	echo "Compilation successful"
	echo -e -n "${NC}"

	./main.out "$@"
else
	echo -e -n "${RED}"
	echo "Compilation failed"
	echo -e -n "${NC}"
fi
