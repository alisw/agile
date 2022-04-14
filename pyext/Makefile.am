if ENABLE_PYEXT

SUBDIRS = AGILe .

all-local: AGILe/agilewrap_wrap.cc
	$(PYTHON) setup.py build

install-exec-local:
	$(PYTHON) setup.py install --prefix=$(DESTDIR)$(prefix)

## TODO: Really want DESTDIR here?
uninstall-local:
	rm -rf $(DESTDIR)$(AGILE_PYTHONPATH)/*AGILe*

clean-local:
	$(PYTHON) setup.py clean --all
	@rm -f $(top_builddir)/*.pyc
	@rm -rf $(builddir)/build
	@rm -rf dist

distclean-local:
	@rm -rf AGILe.egg-info

endif
