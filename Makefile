.PHONY: clean all Release

all:
	make -C Source all

clean:
	make -C Source clean

Release: all
	scp ReadMe.md Source/bin/* jptaylor@frs.sourceforge.net:/home/frs/project/gerber2pdf/Drill2Gerber/
#-------------------------------------------------------------------------------

