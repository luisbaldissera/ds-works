all:
	@echo "Please, run 'make integ' or 'make mpinteg'"

integ:
	$(MAKE) -C integrate

mpinteg:
	$(MAKE) -C mpinteg
