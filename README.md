# WebRTC OpenCV video streaming

## Install Dependencies

### Build ACE(ADAPTIVE Communication Environment)
1. Download zip: [ACE source](https://download.dre.vanderbilt.edu/)
2. Installation: [ACE installation](https://www.dre.vanderbilt.edu/~schmidt/DOC_ROOT/ACE/ACE-INSTALL.html#unix)

#### Problems with ACE
1. Error in building gperf.
Choose build only sources: `cd ace && make`
2. Error in `make install`.
Insert sudo in perl script or uses it:
```
perl -ne'if (/^#END MPC-Generated Install Info/) {exit 0}' \
     -e'elsif (/^#BEGIN MPC-Generated Install Info/) {$ok=1}' \
     -e'elsif ($ok && s/^#//) {print}' GNUmakefile.ACE | \
sudo /home/user/Library/ACE_wrappers/MPC/prj_install.pl -i \
     -s lib_output,header_files,template_files,inline_files,installdata_files \
     -b header_files=include/ace -b template_files=include/ace -b inline_files=include/ace \
     -o lib_output=lib -x installdata_files -b installdata_files=share/ace /usr/local
```
