sponza="testdata/Sponza";
san_miguel="testdata/SanMiguel";
rungholt="testdata/Rungholt";

#frametimes
culling=$(cat "$sponza/Culling/FrameTimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$sponza/AverageFrameTime.dat"
nculling=$(cat "$sponza/NoCulling/FrameTimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "2 \"Without culling\" $nculling" >> "$sponza/AverageFrameTime.dat"

culling=$(cat "$san_miguel/Culling/FrameTimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$san_miguel/AverageFrameTime.dat"
nculling=$(cat "$san_miguel/NoCulling/FrameTimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "2 \"Without culling\" $nculling" >> "$san_miguel/AverageFrameTime.dat"

culling=$(cat "$rungholt/Culling/FrameTimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$rungholt/AverageFrameTime.dat"
nculling=$(cat "$rungholt/NoCulling/FrameTimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "2 \"Without culling\" $nculling" >> "$rungholt/AverageFrameTime.dat"

#CPU time
culling=$(cat "$sponza/Culling/CPU_Timings.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$sponza/AverageCPUTime.dat"
nculling=$(cat "$sponza/NoCulling/CPU_Timings.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "2 \"Without culling\" $nculling" >> "$sponza/AverageCPUTime.dat"

culling=$(cat "$san_miguel/Culling/CPU_Timings.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$san_miguel/AverageCPUTime.dat"
nculling=$(cat "$san_miguel/NoCulling/CPU_Timings.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "2 \"Without culling\" $nculling" >> "$san_miguel/AverageCPUTime.dat"

culling=$(cat "$rungholt/Culling/CPU_Timings.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$rungholt/AverageCPUTime.dat"
nculling=$(cat "$rungholt/NoCulling/CPU_Timings.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "2 \"Without culling\" $nculling" >> "$rungholt/AverageCPUTime.dat"

#Culling time
culling=$(cat "$sponza/Culling/Cullingtimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$sponza/AverageCullingTime.dat"

culling=$(cat "$san_miguel/Culling/Cullingtimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$san_miguel/AverageCullingTime.dat"

culling=$(cat "$rungholt/Culling/Cullingtimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$rungholt/AverageCullingTime.dat"

#render time
culling=$(cat "$sponza/Culling/Rendertimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$sponza/AverageRenderTime.dat"

culling=$(cat "$san_miguel/Culling/Rendertimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$san_miguel/AverageRenderTime.dat"

culling=$(cat "$rungholt/Culling/Rendertimes.dat" | awk '{sum+=$1;n++} END {if(n>0) print sum / n}')
echo "1 Culling $culling" > "$rungholt/AverageRenderTime.dat"
