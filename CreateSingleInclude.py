#!/usr/bin/env python
# This file is heavily derived from Catch2's generateSingleHeader.py... as in, 95%+ from there. 

import os
import io
import re
import datetime

def generate():
    includesParser = re.compile( r'\s*#\s*include\s*"(.*)"' )

    rootPath = 'HippoMocks/'
    outputPath = 'SingleInclude/hippomocks.h'

    outDir = os.path.dirname(outputPath)
    if not os.path.exists(outDir):
        os.makedirs(outDir)
    out = io.open( outputPath, 'w', newline='\n')

    def write( line ):
        out.write( line.decode('utf-8') )

    def parseFile( filename ):
        f = open( filename, 'r' )
        blanks = 0
        write( "// start {0}\n".format( filename ) )
        for line in f:
            m = includesParser.match( line )
            if m:
                header = m.group(1)
                if os.path.exists( rootPath + header ):
                    parseFile( rootPath + header )
                else:
                    write( line.rstrip() + "\n" )
            else:
                write( line.rstrip() + "\n" )
        write( '// end {0}\n'.format(filename) )

    write( "/*\n" )
    write( " *  This file has been merged from multiple headers. Please don't edit it directly\n" )
    write( " *\n" )
    write( " *  Generated: {0}\n".format( datetime.datetime.now() ) )
    write( " */\n" )

    parseFile( rootPath + 'hippomocks.h' )

    write( "#endif\n\n" )
    out.close()

if __name__ == '__main__':
    generate()



