# spice-gui
Graphical User Interface (GUI) including live spike monitoring and control of a simulated cerebellum running on the massively-parallel computer architecture SpiNNaker

###Install & Setup:
- Install Qt5: http://qt-project.org/wiki/Install_Qt_5_on_Ubuntu (e.g., qt-opensource-linux-x64-5.3.2.run)
- Set rights for /dev/ttyUSB0 etc. to user who will start our software: "sudo adduser user_name dialout", then reboot/logout
- In your custom pacman.cfg (for current SpiNNaker-Packages, see .spynnaker.cfg):
  - for [Reports] set defaultReportFilePath and defaultApplicationDataFilePath to according absolute paths
  - for [Recording] set send_live_spikes = True
  - for [Database] set create_database = True (and wait_on_confirmation = True ??)
- After starting the GUI, set the spynnaker.cfg path correcty in the settings. Press STRG+H manually to show hidden files (Qt cannot do it by default!)
  - for OS X: http://macs.about.com/od/usingyourmac/qt/opensavehide.htm

###Switch between old/new SpiNNaker-Package-Support:
- Attention: Support for new SpiNNacker package is in pre-alpha state!
- in settingsdialog.cpp in the GUI: go through the TODO's and comment in/out to switch between old/new spinnaker package
