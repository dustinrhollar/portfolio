
# Get the directory
#ME="$(readlink -f "$0")"
#DIR="$(dirname "$ME")"
#LOCATION="$(dirname "$DIR")"


# Get the target machine
MACHINE=$(./scripts/machine.sh)


if [[ "$MACHINE" == "Linux" ]]; then
	COPY="cp"
	COPYR="cp -r"

	# Get the directory
	ME="$(readlink -f "$0")"
	DIR="$(dirname "$ME")"
	LOCATION="$(dirname "$DIR")"

elif [[ "$MACHINE" == "Mac" ]]; then
	COPYR="gcp"
	COPYR="gcp"
	
	# Get the directory
	ME="$(greadlink -f "$0")"
	DIR="$(dirname "$ME")"
	LOCATION="$(dirname "$DIR")"

fi 

func_check_cmake() {
	if ! cmake &> /dev/null; then
		tput setaf 1;
		echo "################################################################"
		echo "##################  CMAKE NOT INSTALLED. PLEASE INSTALL CMAKE."
		echo "##################  EXITING..."
    	echo "################################################################"
		echo;tput sgr0
		set -e
		exit 1;
	fi
}

func_check_make() {
	if ! make -v COMMAND &> /dev/null; then
		tput setaf 1;
		echo "################################################################"
		echo "##################  MAKEFILES NOT INSTALLED. PLEASE INSTALL MAKE."
		echo "##################  EXITING..."
    	echo "################################################################"
		echo;tput sgr0
		set -e
		exit 1;
	fi
}

tput setaf 5;
echo "################################################################"
echo "##################  Installing GLFW "
echo "################################################################"
echo;tput sgr0

func_check_cmake
func_check_make

echo;

pushd $LOCATION/bin/lib/
	git clone https://github.com/glfw/glfw.git
	mkdir glfw/build
	pushd glfw/build
		cmake -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF ..
		make
	popd
	
	tput setaf 5;
	echo "################################################################"
	echo "##################  CLEANING GLFW "
    echo "################################################################"
	echo;tput sgr0
		
	$COPY glfw/build/src/libglfw* .
	$COPYR glfw/include/GLFW ../include/
	rm -rf glfw/*
	mv libglfw* glfw/
popd

echo;