import sys, string, os

inFileName = sys.argv[1]
outFileName = sys.argv[2]

findStr = sys.argv[3]                                 
replaceStr = sys.argv[4]

inFile = open(inFileName, 'r')
inFileStr = inFile.read()
inFile.close() 

outputStr = inFileStr.replace(findStr, replaceStr) #Find & Replace
       
outFile = open(outFileName, 'w')
outFile.write(outputStr)
outFile.close()