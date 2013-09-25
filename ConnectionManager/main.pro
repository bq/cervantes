
TEMPLATE = subdirs
SUBDIRS = lib connect offline
connect.depends = lib
offline.depends = lib
FAKE_WIFI = $$(FAKE_WIFI)
