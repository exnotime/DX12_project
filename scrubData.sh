#first frames will have some zeroes since there is no data in the buffers. 
echo "scrubadubdub data(removing zeroes)"

sponza="testdata/Sponza";
san_miguel="testdata/SanMiguel";
rungholt="testdata/Rungholt";
#this list should match the testnames in Tests.as or it will not work
declare -a tests=("WarmUp" "Culling" "NoCulling" "1Tri" "2Tri" "3Tri" "4Tri" "Backface" "SmallTris" "Frustum" "Occlusion" "AllFilters" "BatchSize128" "BatchSize256" \
	"BatchSize512" "BatchSize1024" "BatchCount128" "BatchCount256" "BatchCount512" "BatchCount1024" "BatchCount2048" "BatchCount4096" "Resolution270" \
	"Resolution540" "Resolution1080" "Resolution2160");


for t in "${tests[@]}"
do
	for f in "$sponza/$t/"*.dat
	do
		awk '$0 > 0' $f > "$f.scrub";
		cp "$f.scrub" "$f"
		rm "$f.scrub"
	done
	for f in "$san_miguel/$t/"*.dat
	do
		awk '$0 > 0' $f > "$f.scrub";
		cp "$f.scrub" "$f"
		rm "$f.scrub"
	done
		for f in "$rungholt/$t/"*.dat
	do
		awk '$0 > 0' $f > "$f.scrub";
		cp "$f.scrub" "$f"
		rm "$f.scrub"
	done

done