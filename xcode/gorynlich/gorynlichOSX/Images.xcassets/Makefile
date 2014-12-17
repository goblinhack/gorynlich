# Windows       16x16, 32x32, 48x48,   and 256x256
# Mac OS X 	16x16, 32x32, 128x128, and 512x512
# Linux 	16x16, 24x24, 48x48,   and 96x96
# iPhone 	57x57, 72x72, 114x114, 512x512
ORIG=gorynlich.png

gorynlich_114x114.png:
	convert -scale 114x114 $(ORIG) gorynlich_114x114.png

gorynlich_128x128.png:
	convert -scale 128x128 $(ORIG) gorynlich_128x128.png

gorynlich_16x16.png:
	convert -scale 16x16 $(ORIG) gorynlich_16x16.png

gorynlich_24x24.png:
	convert -scale 24x24 $(ORIG) gorynlich_24x24.png

gorynlich_29x29.png:
	convert -scale 29x29 $(ORIG) gorynlich_29x29.png

gorynlich_256x256.png:
	convert -scale 256x256 $(ORIG) gorynlich_256x256.png

gorynlich_512x512.png:
	convert -scale 512x512 $(ORIG) gorynlich_512x512.png

gorynlich_32x32.png:
	convert -scale 32x32 $(ORIG) gorynlich_32x32.png

gorynlich_48x48.png:
	convert -scale 48x48 $(ORIG) gorynlich_48x48.png

gorynlich_50x50.png:
	convert -scale 50x50 $(ORIG) gorynlich_50x50.png

gorynlich_58x58.png:
	convert -scale 58x58 $(ORIG) gorynlich_58x58.png

gorynlich_512x512.icns:
	convert -scale 512x512 $(ORIG) gorynlich_512x512.png
	makeicns -512 gorynlich_512x512.png

gorynlich_57x57.png:
	convert -scale 57x57 $(ORIG) gorynlich_57x57.png

gorynlich_72x72.png:
	convert -scale 72x72 $(ORIG) gorynlich_72x72.png

gorynlich_96x96.png:
	convert -scale 96x96 $(ORIG) gorynlich_96x96.png

iTunesArtwork:
	cp gorynlich_512x512.png iTunesArtwork 	

Icon.png:
	cp gorynlich_57x57.png Icon.png 	

Icon@2x.png:
	cp gorynlich_114x114.png Icon@2x.png 	

Icon-72.png:
	cp gorynlich_72x72.png Icon-72.png 	

Icon-Small.png:
	cp gorynlich_29x29.png Icon-Small.png 	

Icon-Small-50.png:
	cp gorynlich_50x50.png Icon-Small-50.png 	

Icon-Small@2x.png:
	cp gorynlich_58x58.png Icon-Small@2x.png

TARGETS=gorynlich_114x114.png gorynlich_128x128.png gorynlich_16x16.png gorynlich_29x29.png gorynlich_24x24.png gorynlich_256x256.png gorynlich_32x32.png gorynlich_48x48.png gorynlich_512x512.png gorynlich_57x57.png gorynlich_72x72.png gorynlich_96x96.png gorynlich_50x50.png gorynlich_58x58.png iTunesArtwork Icon.png Icon@2x.png Icon-72.png Icon-Small.png Icon-Small-50.png Icon-Small@2x.png gorynlich_512x512.icns 

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
