
export BUILD_TYPE=Debug
if [ "$1" == 'Release' ];
then
	export BUILD_TYPE=Release
fi

export BUILD_FOLDER=_build_"$BUILD_TYPE"

if [ ! -d $BUILD_FOLDER ];
then
	mkdir $BUILD_FOLDER
fi

cd $BUILD_FOLDER
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBOOST_HEADER_ONLY=ON -DVERBOSE=OFF -G "Unix Makefiles" ../ 
cd ../

