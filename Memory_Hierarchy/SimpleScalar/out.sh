echo " sequential read "
echo "arraysize       exectime       il1hitrate          dl1hitrate          ul2hitrate"
for i in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768
do	
	cycles=$(cat ~/cse420/seqread/$i.txt | grep "sim_cycle" | awk '{print $2}')
	il1accesses=$(cat ~/cse420/seqread/$i.txt | grep "il1.accesses" | awk '{print $2}')
	il1hits=$(cat ~/cse420/seqread/$i.txt | grep "il1.hits" | awk '{print $2}') 
	dl1accesses=$(cat ~/cse420/seqread/$i.txt | grep "dl1.accesses" | awk '{print $2}')
	dl1hits=$(cat ~/cse420/seqread/$i.txt | grep "dl1.hits" | awk '{print $2}') 
	ul2accesses=$(cat ~/cse420/seqread/$i.txt | grep "ul2.accesses" | awk '{print $2}')
	ul2hits=$(cat ~/cse420/seqread/$i.txt | grep "ul2.hits" | awk '{print $2}')	
	
	x=$(bc -l <<< " $cycles / 100 ")
	etime=$(bc -l <<< " $x / $i ")
	
	ih=$(bc -l <<< " $il1hits / $il1accesses ")
	dh=$(bc -l <<< " $dl1hits / $dl1accesses ")
	uh=$(bc -l <<< " $ul2hits / $ul2accesses ")
	echo $i $etime $ih $dh $uh
		
done


echo " sequential write "
echo "arraysize       exectime       il1hitrate          dl1hitrate          ul2hitrate"
for i in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768
do	
	cycles=$(cat ~/cse420/seqwrite/$i.txt | grep "sim_cycle" | awk '{print $2}')
	il1accesses=$(cat ~/cse420/seqwrite/$i.txt | grep "il1.accesses" | awk '{print $2}')
	il1hits=$(cat ~/cse420/seqwrite/$i.txt | grep "il1.hits" | awk '{print $2}') 
	dl1accesses=$(cat ~/cse420/seqwrite/$i.txt | grep "dl1.accesses" | awk '{print $2}')
	dl1hits=$(cat ~/cse420/seqwrite/$i.txt | grep "dl1.hits" | awk '{print $2}') 
	ul2accesses=$(cat ~/cse420/seqwrite/$i.txt | grep "ul2.accesses" | awk '{print $2}')
	ul2hits=$(cat ~/cse420/seqwrite/$i.txt | grep "ul2.hits" | awk '{print $2}')	
	
	x=$(bc -l <<< " $cycles / 100 ")
	etime=$(bc -l <<< " $x / $i ")
	
	ih=$(bc -l <<< " $il1hits / $il1accesses ")
	dh=$(bc -l <<< " $dl1hits / $dl1accesses ")
	uh=$(bc -l <<< " $ul2hits / $ul2accesses ")
	echo $i $etime $ih $dh $uh
		
done


echo " random read "
echo "arraysize       exectime       il1hitrate          dl1hitrate          ul2hitrate"
for i in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768
do	
	cycles=$(cat ~/cse420/ranread/$i.txt | grep "sim_cycle" | awk '{print $2}')
	il1accesses=$(cat ~/cse420/ranread/$i.txt | grep "il1.accesses" | awk '{print $2}')
	il1hits=$(cat ~/cse420/ranread/$i.txt | grep "il1.hits" | awk '{print $2}') 
	dl1accesses=$(cat ~/cse420/ranread/$i.txt | grep "dl1.accesses" | awk '{print $2}')
	dl1hits=$(cat ~/cse420/ranread/$i.txt | grep "dl1.hits" | awk '{print $2}') 
	ul2accesses=$(cat ~/cse420/ranread/$i.txt | grep "ul2.accesses" | awk '{print $2}')
	ul2hits=$(cat ~/cse420/ranread/$i.txt | grep "ul2.hits" | awk '{print $2}')	
	
	x=$(bc -l <<< " $cycles / 100 ")
	etime=$(bc -l <<< " $x / $i ")
	
	ih=$(bc -l <<< " $il1hits / $il1accesses ")
	dh=$(bc -l <<< " $dl1hits / $dl1accesses ")
	uh=$(bc -l <<< " $ul2hits / $ul2accesses ")
	echo $i $etime $ih $dh $uh
		
done


echo " random write "
echo "arraysize       exectime       il1hitrate          dl1hitrate          ul2hitrate"
for i in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768
do	
	cycles=$(cat ~/cse420/ranwrite/$i.txt | grep "sim_cycle" | awk '{print $2}')
	il1accesses=$(cat ~/cse420/ranwrite/$i.txt | grep "il1.accesses" | awk '{print $2}')
	il1hits=$(cat ~/cse420/ranwrite/$i.txt | grep "il1.hits" | awk '{print $2}') 
	dl1accesses=$(cat ~/cse420/ranwrite/$i.txt | grep "dl1.accesses" | awk '{print $2}')
	dl1hits=$(cat ~/cse420/ranwrite/$i.txt | grep "dl1.hits" | awk '{print $2}') 
	ul2accesses=$(cat ~/cse420/ranwrite/$i.txt | grep "ul2.accesses" | awk '{print $2}')
	ul2hits=$(cat ~/cse420/ranwrite/$i.txt | grep "ul2.hits" | awk '{print $2}')	
	
	x=$(bc -l <<< " $cycles / 100 ")
	etime=$(bc -l <<< " $x / $i ")
	
	ih=$(bc -l <<< " $il1hits / $il1accesses ")
	dh=$(bc -l <<< " $dl1hits / $dl1accesses ")
	uh=$(bc -l <<< " $ul2hits / $ul2accesses ")
	echo $i $etime $ih $dh $uh
		
done



