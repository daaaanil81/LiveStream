DIR_SOURCE 	= source
DIR_CER 	= certificate	
MAKE 		= make

compile:
	$(MAKE) -C $(DIR_SOURCE)
certificate:
	$(MAKE) -C $(DIR_CER)
clean:
	$(MAKE) -C $(DIR_SOURCE) clean
	- rm -r *~
