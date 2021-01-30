.PHONY: clean all release

all:
	$(MAKE) -C Source all

clean:
	$(MAKE) -C Source clean

release: all
	scp ReadMe.md Source/bin/* jptaylor@frs.sourceforge.net:/home/frs/project/gerber2pdf/Drill2Gerber/
#-------------------------------------------------------------------------------

