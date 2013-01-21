#Directory configuration
APIDIR = ../api
NUGGETSDIR=nuggets

#Original nugget directory names
CLAMAVNUGGETORIG = clamavNugget
VIRUSTOTALORIG = virusTotal
SWFSCANNERORIG = swfScanner
OFFICECATORIG = officeCat
ARCHIVEINFLATEORIG = archiveInflate
PDFFOXORIG = pdfFox

#Target directory names
SHORTCLAMAVNUGGET = clamavnugget
SHORTVIRUSTOTAL = virustotal
SHORTSWFSCANNER = swfscanner
SHORTOFFICECAT = officecat
SHORTARCHIVEINFLATE = archiveinflate
SHORTPDFFOX = pdffox

#Target directory names
CLAMAVNUGGET = rb-$(SHORTCLAMAVNUGGET)
VIRUSTOTAL = rb-$(SHORTVIRUSTOTAL)
SWFSCANNER = rb-$(SHORTSWFSCANNER)
OFFICECAT = rb-$(SHORTOFFICECAT)
ARCHIVEINFLATE = rb-$(SHORTARCHIVEINFLATE)
PDFFOX = rb-$(SHORTPDFFOX)

#Defines used in compliation
WRAPPERDIR = src
DEBUG = -g
EXTRASC = -c api/*.c  -std=gnu99 -I/usr/include/libxml2/ -I$(APIDIR)/include/ $(DEBUG)   -D_GNU_SOURCE -fno-strict-aliasing
CC = cc
.PHONY: all clean scriptNugget swfScanner clamavNugget pdfFox yaraNugget virusTotal officeCat archiveInflate test tests nugget-commons scriptNuggetPKG swfScannerPKG clamavNuggetPKG pdfFoxPKG yaraNuggetPKG virusTotalPKG officeCatPKG archiveInflatePKG

#Libraries required for each nugget
SHAREDLIBS = -lconfig -luuid -lpthread -lrt -lssl
CLAMAVLIBS = $(SHAREDLIBS) -lconfig  
VIRUSTOTALLIBS = $(SHAREDLIBS) -lcurl  -ljson
ARCHIVEINFLATELIBS =  $(SHAREDLIBS) -larchive
SWFSCANNERLIBS = $(SHAREDLIBS) -lm
PDFFOXLIBS = $(SHAREDLIBS) -lm
OFFICECATLIBS = $(SHAREDLIBS)

TARGETLOCATION = /opt/hsn2
CONFTARGETLOCATION = /etc/hsn2/razorback
DEBIAN_DIST=experimental

# scriptNugget - works, but not interesting - requires scripts anyhow.
# yaraNugget - seg faults - better to implement on our own either way.
all: swfScanner clamavNugget officeCat pdfFox virusTotal archiveInflate 

packages32: swfScannerPKG32 clamavNuggetPKG32 pdfFoxPKG32 virusTotalPKG32 archiveInflatePKG32 officeCatPKG32
packages64: swfScannerPKG64 clamavNuggetPKG64 pdfFoxPKG64 virusTotalPKG64 archiveInflatePKG64 officeCatPKG64

# Each nugget has 3 build rules:
# - a rule to compile the nugget
# - a rule to build the i386 package (suffix PKG32)
# - a rule to build the amd64 package (suffix PKG64)

clamavNugget:
	rm -rf $(CLAMAVNUGGET)
	cp -rf $(WRAPPERDIR) $(CLAMAVNUGGET)
	cp -rf $(NUGGETSDIR)/$(CLAMAVNUGGETORIG)/src/* ./$(CLAMAVNUGGET)/
	cd $(CLAMAVNUGGET); cc *.c $(EXTRASC)
	cd $(CLAMAVNUGGET); cc *.o -o $(CLAMAVNUGGET) $(CLAMAVLIBS)
	cd $(CLAMAVNUGGET); rm -f *.o
	#mv ./$(CLAMAVNUGGET)/$(CLAMAVNUGGETORIG) ./$(CLAMAVNUGGET)/$(CLAMAVNUGGET)

nugget-commons:
	$(eval NUGGET := nugget-commons)
	$(eval TARGET := $(NUGGET)$(TARGETLOCATION)/python/$(NUGGET))
	$(eval CONFTARGET := $(NUGGET)$(CONFTARGETLOCATION))
	$(eval DEBIANTARGET := $(NUGGET)/DEBIAN)
	rm -rf $(NUGGET)PKG;
	mkdir -p $(TARGET);
	mkdir -p $(CONFTARGET)/mappings;
	mkdir -p $(DEBIANTARGET);
	cp ./nuggetService.py $(TARGET)/
	cp ./nuggetTaskProcessor.py $(TARGET)/
	cp ./conf/api.conf $(CONFTARGET)/
	cp ./conf/mappings/default_input.py $(CONFTARGET)/mappings/;
	cp ./conf/mappings/default_output.py $(CONFTARGET)/mappings/;
	cp ./debian/$(NUGGET)/* $(DEBIANTARGET)/
	sed -i "s/{DEBIAN_DIST}/${DEBIAN_DIST}/" $(DEBIANTARGET)/control
	sed -i "s/{VER}/${HSN2_VER}-${BUILD_NUMBER}/" $(DEBIANTARGET)/control
	fakeroot dpkg -b $(NUGGET)
	mv $(NUGGET).deb hsn2-rb-$(NUGGET)-${HSN2_VER}-${BUILD_NUMBER}_all.deb

setArch32:
	$(eval ARCH := i386)

setArch64:
	$(eval ARCH := amd64)
	
clamavNuggetPKG32: setArch32 clamavNuggetPKG
clamavNuggetPKG64: setArch64 clamavNuggetPKG
clamavNuggetPKG: clamavNugget
	$(eval NUGGET := $(CLAMAVNUGGET))
	$(eval TARGET := $(NUGGET)PKG$(TARGETLOCATION)/$(NUGGET))
	$(eval CONFTARGET := $(NUGGET)PKG$(CONFTARGETLOCATION))
	$(eval DEBIANTARGET := $(NUGGET)PKG/DEBIAN)
	$(eval ETCTARGET := $(NUGGET)PKG/etc/init.d)
	rm -rf $(NUGGET)PKG
	mkdir -p $(ETCTARGET);
	mkdir -p $(TARGET);
	mkdir -p $(CONFTARGET)/mappings;
	mkdir -p $(DEBIANTARGET);
	cp $(NUGGET)/$(NUGGET) $(TARGET)/
	cp ./conf/clamav.conf $(CONFTARGET)/
	cp ./conf/mappings/$(SHORTCLAMAVNUGGET)_output.py $(CONFTARGET)/mappings/
	cp ./debian/$(NUGGET)/* $(DEBIANTARGET)/
	chmod 755 $(DEBIANTARGET)/postrm
	rm -f $(DEBIANTARGET)/initd
	$(eval ETCTARGET := $(NUGGET)PKG/etc/init.d)
	mkdir -p $(ETCTARGET);
	cp ./debian/$(NUGGET)/initd $(ETCTARGET)/hsn2-$(NUGGET)
	sed -i "s/{ARCH}/$(ARCH)/" $(DEBIANTARGET)/control
	sed -i "s/{DEBIAN_DIST}/${DEBIAN_DIST}/" $(DEBIANTARGET)/control
	sed -i "s/{VER}/${HSN2_VER}-${BUILD_NUMBER}/" $(DEBIANTARGET)/control
	fakeroot dpkg -b $(NUGGET)PKG
	mv $(NUGGET)PKG.deb hsn2-$(NUGGET)-${HSN2_VER}-${BUILD_NUMBER}_$(ARCH).deb
	
swfScanner:
	rm -rf $(SWFSCANNER)
	cp -rf $(WRAPPERDIR) $(SWFSCANNER)
	cp -rf $(NUGGETSDIR)/$(SWFSCANNERORIG)/src/* ./$(SWFSCANNER)/
	cd $(SWFSCANNER); cc  *.c $(EXTRASC)
	cd $(SWFSCANNER); cc *.o -o $(SWFSCANNER) $(SWFSCANNERLIBS)
	cd $(SWFSCANNER); rm -f *.o
	#mv ./$(SWFSCANNER)/$(SWFSCANNERORIG) ./$(SWFSCANNER)/$(SWFSCANNER)

swfScannerPKG32: setArch32 swfScannerPKG
swfScannerPKG64: setArch64 swfScannerPKG
swfScannerPKG: swfScanner
	$(eval NUGGET := $(SWFSCANNER))
	$(eval TARGET := $(NUGGET)PKG$(TARGETLOCATION)/$(NUGGET))
	$(eval CONFTARGET := $(NUGGET)PKG$(CONFTARGETLOCATION))
	$(eval DEBIANTARGET := $(NUGGET)PKG/DEBIAN)
	rm -rf $(NUGGET)PKG
	mkdir -p $(TARGET);
	mkdir -p $(CONFTARGET)/mappings;
	mkdir -p $(DEBIANTARGET);
	cp $(NUGGET)/$(NUGGET) $(TARGET)/
	cp ./conf/swf_scanner.conf $(CONFTARGET)/
	cp ./conf/mappings/$(SHORTSWFSCANNER)_output.py $(CONFTARGET)/mappings/
	cp ./debian/$(NUGGET)/* $(DEBIANTARGET)/
	chmod 755 $(DEBIANTARGET)/postrm
	rm -f $(DEBIANTARGET)/initd
	$(eval ETCTARGET := $(NUGGET)PKG/etc/init.d)
	mkdir -p $(ETCTARGET);
	cp ./debian/$(NUGGET)/initd $(ETCTARGET)/hsn2-$(NUGGET)
	sed -i "s/{ARCH}/$(ARCH)/" $(DEBIANTARGET)/control
	sed -i "s/{DEBIAN_DIST}/${DEBIAN_DIST}/" $(DEBIANTARGET)/control
	sed -i "s/{VER}/${HSN2_VER}-${BUILD_NUMBER}/" $(DEBIANTARGET)/control
	fakeroot dpkg -b $(NUGGET)PKG
	mv $(NUGGET)PKG.deb hsn2-$(NUGGET)-${HSN2_VER}-${BUILD_NUMBER}_$(ARCH).deb

officeCat:
	rm -rf $(OFFICECAT)
	cp -rf $(WRAPPERDIR) $(OFFICECAT)
	cp -rf $(NUGGETSDIR)/$(OFFICECATORIG)/src/* ./$(OFFICECAT)/
	cp -rf $(NUGGETSDIR)/$(OFFICECATORIG)/officecat ./$(OFFICECAT)/officecat
	cd $(OFFICECAT); cc  *.c $(EXTRASC)
	cd $(OFFICECAT); cc *.o -o $(OFFICECAT) $(OFFICECATLIBS)
	cd $(OFFICECAT); rm -f *.o
	#mv ./$(OFFICECAT)/$(OFFICECATORIG) ./$(OFFICECAT)/$(OFFICECAT)

officeCatPKG: officeCat
	$(eval NUGGET := $(OFFICECAT))
	$(eval TARGET := $(NUGGET)PKG$(TARGETLOCATION)/$(NUGGET))
	$(eval CONFTARGET := $(NUGGET)PKG$(CONFTARGETLOCATION))
	$(eval DEBIANTARGET := $(NUGGET)PKG/DEBIAN)
	rm -rf $(NUGGET)PKG
	mkdir -p $(TARGET);
	mkdir -p $(CONFTARGET)/mappings;
	mkdir -p $(DEBIANTARGET);
	cp $(NUGGET)/$(NUGGET) $(TARGET)/
	cp -r $(NUGGET)/officecat $(TARGET)/officecat
	cp ./conf/officecat.conf $(CONFTARGET)/
	cp ./conf/mappings/$(SHORTOFFICECAT)_output.py $(CONFTARGET)/mappings/
	cp ./debian/$(NUGGET)/* $(DEBIANTARGET)/
	chmod 755 $(DEBIANTARGET)/postrm
	rm -f $(DEBIANTARGET)/initd
	$(eval ETCTARGET := $(NUGGET)PKG/etc/init.d)
	mkdir -p $(ETCTARGET);
	cp ./debian/$(NUGGET)/initd $(ETCTARGET)/hsn2-$(NUGGET)
	chmod +x $(ETCTARGET)/hsn2-$(NUGGET)
	sed -i "s/{ARCH}/$(ARCH)/" $(DEBIANTARGET)/control
	sed -i "s/{DEBIAN_DIST}/${DEBIAN_DIST}/" $(DEBIANTARGET)/control
	sed -i "s/{VER}/${HSN2_VER}-${BUILD_NUMBER}/" $(DEBIANTARGET)/control
	fakeroot dpkg -b $(NUGGET)PKG
	mv $(NUGGET)PKG.deb hsn2-$(NUGGET)-${HSN2_VER}-${BUILD_NUMBER}_$(ARCH).deb

officeCatPKG32: setArch32 officeCatPKG
officeCatPKG64: setArch64 officeCatPKG

pdfFox:
	rm -rf $(PDFFOX)
	cp -rf $(WRAPPERDIR) $(PDFFOX)
	cp -rf $(NUGGETSDIR)/$(PDFFOXORIG)/src/* ./$(PDFFOX)/
	cd $(PDFFOX); cc  *.c $(EXTRASC)
	cd $(PDFFOX); cc *.o -o $(PDFFOX) $(PDFFOXLIBS)
	cd $(PDFFOX); rm -f *.o
	#mv ./$(PDFFOX)/$(PDFFOXORIG) ./$(PDFFOX)/$(PDFFOX)

pdfFoxPKG32: setArch32 pdfFoxPKG
pdfFoxPKG64: setArch64 pdfFoxPKG
pdfFoxPKG: pdfFox
	$(eval NUGGET := $(PDFFOX))
	$(eval TARGET := $(NUGGET)PKG$(TARGETLOCATION)/$(NUGGET))
	$(eval CONFTARGET := $(NUGGET)PKG$(CONFTARGETLOCATION))
	$(eval DEBIANTARGET := $(NUGGET)PKG/DEBIAN)
	rm -rf $(NUGGET)PKG
	mkdir -p $(TARGET);
	mkdir -p $(CONFTARGET)/mappings;
	mkdir -p $(DEBIANTARGET);
	cp $(NUGGET)/$(NUGGET) $(TARGET)/
	cp ./conf/pdffox.conf $(CONFTARGET)/
	cp ./conf/mappings/$(SHORTPDFFOX)_output.py $(CONFTARGET)/mappings/
	cp ./debian/$(NUGGET)/* $(DEBIANTARGET)/
	chmod 755 $(DEBIANTARGET)/postrm
	rm -f $(DEBIANTARGET)/initd
	$(eval ETCTARGET := $(NUGGET)PKG/etc/init.d)
	mkdir -p $(ETCTARGET);
	cp ./debian/$(NUGGET)/initd $(ETCTARGET)/hsn2-$(NUGGET)
	sed -i "s/{ARCH}/$(ARCH)/" $(DEBIANTARGET)/control
	sed -i "s/{DEBIAN_DIST}/${DEBIAN_DIST}/" $(DEBIANTARGET)/control
	sed -i "s/{VER}/${HSN2_VER}-${BUILD_NUMBER}/" $(DEBIANTARGET)/control
	fakeroot dpkg -b $(NUGGET)PKG
	mv $(NUGGET)PKG.deb hsn2-$(NUGGET)-${HSN2_VER}-${BUILD_NUMBER}_$(ARCH).deb
	
virusTotal:
	rm -rf $(VIRUSTOTAL)
	cp -rf $(WRAPPERDIR) $(VIRUSTOTAL)
	cp -rf $(NUGGETSDIR)/$(VIRUSTOTALORIG)/src/* ./$(VIRUSTOTAL)/
	cd $(VIRUSTOTAL); cc  *.c $(EXTRASC) 
	cd $(VIRUSTOTAL); cc *.o -o $(VIRUSTOTAL) $(VIRUSTOTALLIBS)
	cd $(VIRUSTOTAL); rm -f *.o
	#mv ./$(VIRUSTOTAL)/$(VIRUSTOTALORIG) ./$(VIRUSTOTAL)/$(VIRUSTOTAL)

virusTotalPKG32: setArch32 virusTotalPKG
virusTotalPKG64: setArch64 virusTotalPKG
virusTotalPKG: virusTotal
	$(eval NUGGET := $(VIRUSTOTAL))
	$(eval TARGET := $(NUGGET)PKG$(TARGETLOCATION)/$(NUGGET))
	$(eval CONFTARGET := $(NUGGET)PKG$(CONFTARGETLOCATION))
	$(eval DEBIANTARGET := $(NUGGET)PKG/DEBIAN)
	rm -rf $(NUGGET)PKG
	mkdir -p $(TARGET);
	mkdir -p $(CONFTARGET)/mappings;
	mkdir -p $(DEBIANTARGET);
	cp $(NUGGET)/$(NUGGET) $(TARGET)/
	cp ./conf/virustotal.conf $(CONFTARGET)/
	cp ./conf/mappings/$(SHORTVIRUSTOTAL)_output.py $(CONFTARGET)/mappings/
	cp ./debian/$(NUGGET)/* $(DEBIANTARGET)/
	chmod 755 $(DEBIANTARGET)/postrm
	rm -f $(DEBIANTARGET)/initd
	$(eval ETCTARGET := $(NUGGET)PKG/etc/init.d)
	mkdir -p $(ETCTARGET);
	cp ./debian/$(NUGGET)/initd $(ETCTARGET)/hsn2-$(NUGGET)
	sed -i "s/{ARCH}/$(ARCH)/" $(DEBIANTARGET)/control
	sed -i "s/{DEBIAN_DIST}/${DEBIAN_DIST}/" $(DEBIANTARGET)/control
	sed -i "s/{VER}/${HSN2_VER}-${BUILD_NUMBER}/" $(DEBIANTARGET)/control
	fakeroot dpkg -b $(NUGGET)PKG
	mv $(NUGGET)PKG.deb hsn2-$(NUGGET)-${HSN2_VER}-${BUILD_NUMBER}_$(ARCH).deb

archiveInflate:
	rm -rf $(ARCHIVEINFLATE)
	cp -rf $(WRAPPERDIR) $(ARCHIVEINFLATE)
	cp -rf $(NUGGETSDIR)/$(ARCHIVEINFLATEORIG)/src/* ./$(ARCHIVEINFLATE)/
	cd $(ARCHIVEINFLATE); cc  *.c $(EXTRASC)
	cd $(ARCHIVEINFLATE); cc *.o -o $(ARCHIVEINFLATE) $(ARCHIVEINFLATELIBS)
	cd $(ARCHIVEINFLATE); rm -f *.o
	#mv ./$(ARCHIVEINFLATE)/$(ARCHIVEINFLATEORIG) ./$(ARCHIVEINFLATE)/$(ARCHIVEINFLATE)

archiveInflatePKG32: setArch32 archiveInflatePKG
archiveInflatePKG64: setArch64 archiveInflatePKG
archiveInflatePKG: archiveInflate
	$(eval NUGGET := $(ARCHIVEINFLATE))
	$(eval TARGET := $(NUGGET)PKG$(TARGETLOCATION)/$(NUGGET))
	$(eval CONFTARGET := $(NUGGET)PKG$(CONFTARGETLOCATION))
	$(eval DEBIANTARGET := $(NUGGET)PKG/DEBIAN)
	rm -rf $(NUGGET)PKG
	mkdir -p $(TARGET);
	mkdir -p $(CONFTARGET)/mappings;
	mkdir -p $(DEBIANTARGET);
	cp $(NUGGET)/$(NUGGET) $(TARGET)/
	cp ./conf/archive_inflate.conf $(CONFTARGET)/
	cp ./conf/mappings/$(SHORTARCHIVEINFLATE)_output.py $(CONFTARGET)/mappings/
	cp ./debian/$(NUGGET)/* $(DEBIANTARGET)/
	chmod 755 $(DEBIANTARGET)/postrm
	rm -f $(DEBIANTARGET)/initd
	$(eval ETCTARGET := $(NUGGET)PKG/etc/init.d)
	mkdir -p $(ETCTARGET);
	cp ./debian/$(NUGGET)/initd $(ETCTARGET)/hsn2-$(NUGGET)
	sed -i "s/{ARCH}/$(ARCH)/" $(DEBIANTARGET)/control
	sed -i "s/{DEBIAN_DIST}/${DEBIAN_DIST}/" $(DEBIANTARGET)/control
	sed -i "s/{VER}/${HSN2_VER}-${BUILD_NUMBER}/" $(DEBIANTARGET)/control
	fakeroot dpkg -b $(NUGGET)PKG
	mv $(NUGGET)PKG.deb hsn2-$(NUGGET)-${HSN2_VER}-${BUILD_NUMBER}_$(ARCH).deb

clean:
	rm -f *.o
	rm -rf $(SWFSCANNER) $(CLAMAVNUGGET) $(OFFICECAT) $(PDFFOX) $(VIRUSTOTAL) $(ARCHIVEINFLATE)
	rm -rf nugget-commons $(SWFSCANNER)PKG $(CLAMAVNUGGET)PKG $(OFFICECAT)PKG $(PDFFOX)PKG $(VIRUSTOTAL)PKG $(ARCHIVEINFLATE)PKG
	rm -f *.deb
	
test: tests

tests:
	@mkdir -p /tmp/tests/out
	cd test; nosetests -s --with-xunit --xunit-file=/tmp/tests/out/rb-razorbackunittests.xml;
