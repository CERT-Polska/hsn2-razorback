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
EXTRASC = -c api/*.c  -std=gnu99 `pkg-config openssl --cflags` `pkg-config uuid --cflags` `pkg-config libconfig --cflags` `pkg-config zlib --cflags` -I$(APIDIR)/include/ $(DEBUG)   -D_GNU_SOURCE -fno-strict-aliasing
CC = cc
.PHONY: all clean scriptNugget swfScanner clamavNugget pdfFox yaraNugget virusTotal officeCat archiveInflate test tests nugget-commons scriptNuggetPKG swfScannerPKG clamavNuggetPKG pdfFoxPKG yaraNuggetPKG virusTotalPKG officeCatPKG archiveInflatePKG

#Libraries required for each nugget
SHAREDLIBS = -lpthread -lrt `pkg-config openssl --libs` `pkg-config uuid --libs` `pkg-config libconfig --libs`
CLAMAVLIBS = $(SHAREDLIBS) -lconfig
VIRUSTOTALLIBS = $(SHAREDLIBS) -lcurl  -ljson
ARCHIVEINFLATELIBS =  $(SHAREDLIBS) -larchive
SWFSCANNERLIBS = $(SHAREDLIBS) -lm `pkg-config zlib --libs`
PDFFOXLIBS = $(SHAREDLIBS) -lm `pkg-config zlib --libs`
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
	cd $(CLAMAVNUGGET); strip $(CLAMAVNUGGET)
	cd $(CLAMAVNUGGET); rm -f *.o

nugget-commons:
	cd hsn2-nugget-commons; debuild -us -uc; debuild clean

setArch32:
	$(eval ARCH := i386)

setArch64:
	$(eval ARCH := amd64)

clamavNuggetPKG32: setArch32 clamavNuggetPKG
clamavNuggetPKG64: setArch64 clamavNuggetPKG
clamavNuggetPKG: clamavNugget
	mv $(CLAMAVNUGGET)/$(CLAMAVNUGGET) hsn2-clamavnugget/
	cd hsn2-clamavnugget; debuild -a$(ARCH) -us -uc; debuild clean

swfScanner:
	rm -rf $(SWFSCANNER)
	cp -rf $(WRAPPERDIR) $(SWFSCANNER)
	cp -rf $(NUGGETSDIR)/$(SWFSCANNERORIG)/src/* ./$(SWFSCANNER)/
	cd $(SWFSCANNER); cc  *.c $(EXTRASC)
	cd $(SWFSCANNER); cc *.o -o $(SWFSCANNER) $(SWFSCANNERLIBS)
	cd $(SWFSCANNER); rm -f *.o

swfScannerPKG32: setArch32 swfScannerPKG
swfScannerPKG64: setArch64 swfScannerPKG
swfScannerPKG: swfScanner
	mv $(SWFSCANNER)/$(SWFSCANNER) hsn2-swfscanner/
	cd hsn2-swfscanner; debuild -a$(ARCH) -us -uc; debuild clean

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
	mv $(OFFICECAT)/$(OFFICECAT) hsn2-officecat/
	cd hsn2-officecat; debuild -a$(ARCH) -us -uc; debuild clean

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
	mv $(PDFFOX)/$(PDFFOX) hsn2-pdffox/
	cd hsn2-pdffox; debuild -a$(ARCH) -us -uc; debuild clean

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
	mv $(VIRUSTOTAL)/$(VIRUSTOTAL) hsn2-virustotal/
	cd hsn2-virustotal; debuild -a$(ARCH) -us -uc; debuild clean

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
	mv $(ARCHIVEINFLATE)/$(ARCHIVEINFLATE) hsn2-archiveinflate/
	cd hsn2-archiveinflate; debuild -a$(ARCH) -us -uc; debuild clean

clean:
	rm -f *.o
	rm -rf $(SWFSCANNER) $(CLAMAVNUGGET) $(OFFICECAT) $(PDFFOX) $(VIRUSTOTAL) $(ARCHIVEINFLATE)
	rm -rf nugget-commons $(SWFSCANNER)PKG $(CLAMAVNUGGET)PKG $(OFFICECAT)PKG $(PDFFOX)PKG $(VIRUSTOTAL)PKG $(ARCHIVEINFLATE)PKG
	rm -f *.deb *.dsc *.tar.* *.build *.changes
	cd hsn2-archiveinflate; debuild clean
	cd hsn2-clamavnugget; debuild clean
	cd hsn2-nugget-commons; debuild clean
	cd hsn2-officecat; debuild clean
	cd hsn2-pdffox; debuild clean
	cd hsn2-swfscanner; debuild clean
	cd hsn2-virustotal; debuild clean

test: tests

tests:
	@mkdir -p /tmp/tests/out
	cd test; nosetests -s --with-xunit --xunit-file=/tmp/tests/out/rb-razorbackunittests.xml;
