Summary: MSICP, It handles mediaserver calls and routes to registrar 
Name: wsmsicp
Version: 1
Release: RFC3326
License: Under Pandora Newtorks Software Licence.
Group: Servers
%description
%install
install --directory $RPM_BUILD_ROOT/home/wsadmin/wsmsicp
cp -r /home/wsadmin/wsmsicp/* $RPM_BUILD_ROOT/home/wsadmin/wsmsicp/
%files
"/home/wsadmin/wsmsicp/"
