#!/usr/bin/ruby
# 

def printUsage()
  puts "usage: flip_includes file1 [file2 [...]]"
  puts
end


def flip_includes( lines )

  output = []

  for line in lines
    if( line =~ /^#include.*<wg_/ )
    	line.sub! /<wg_/, '<wg3_'
    elsif( line =~ /^#include.*<wg3_/ )
    	line.sub! /<wg3_/, '<wg_'
    elsif( line =~ /WG_\w+_DOT_H/ )
    	line.sub! /WG_/, 'WG3_'
    elsif( line =~ /WG3_\w+_DOT_H/ )
    	line.sub! /WG3_/, 'WG_'
    end
    output << line
  end

  return output
end



if( $*.length < 1 )
  printUsage()
  exit
end

for fileName in $*

  puts "Flipping includes: #{fileName}"

  content = flip_includes( IO.readlines(fileName) )

  f = File.new( fileName, "w")
	f.puts content
	f.close
end
