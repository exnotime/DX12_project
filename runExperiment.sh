#this script will run the experiment for each of the scenes
echo "Starting test" 
date
#create directories
mkdir -p "testdata";
sponza="testdata/Sponza";
san_miguel="testdata/SanMiguel";
rungholt="testdata/Rungholt";
#this list should match the testnames in Tests.as or it will not work
declare -a tests=("WarmUp" "Culling" "NoCulling" "1Tri" "2Tri" "3Tri" "4Tri" "Backface" "SmallTris" "Frustum" "Occlusion" "AllFilters" "BatchSize128" "BatchSize256" \
	"BatchSize512" "BatchSize1024" "BatchCount128" "BatchCount256" "BatchCount512" "BatchCount1024" "BatchCount2048" "BatchCount4096" "Resolution270" \
	"Resolution540" "Resolution1080" "Resolution2160");

mkdir -p "$sponza";
mkdir -p "$san_miguel";
mkdir -p "$rungholt";

for t in "${tests[@]}"
do
	mkdir -p "$sponza/$t";
	mkdir -p "$san_miguel/$t";
	mkdir -p "$rungholt/$t";
done
#copy and original so we can run the test multiple times without having to worry about editing the angelscript file every time
cp ./script/Tests.as ./script/Tests.as.b;
#replace the directory that testdata are placed in so we have the scenes seperated
sed -i -e "s|#REPLACE_THIS_DIR|$sponza|g" ./script/Tests.as;
./Engine.exe SPONZA
cp ./script/Tests.as.b ./script/Tests.as;
sed -i -e "s|#REPLACE_THIS_DIR|$san_miguel|g" ./script/Tests.as;
./Engine.exe SAN_MIGUEL
cp ./script/Tests.as.b ./script/Tests.as;
sed -i -e "s|#REPLACE_THIS_DIR|$rungholt|g" ./script/Tests.as;
./Engine.exe RUNGHOLT

cp ./script/Tests.as.b ./script/Tests.as;
rm ./script/Tests.as.b;
echo "Ending test"
date
read -n 1 -p "Press any key to quit"
