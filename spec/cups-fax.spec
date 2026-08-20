#
# spec file for package cups-fax
#
# Copyright (c) 2026 GTC TeleCommunication GmbH
#
# Copyright (c) 2021 SUSE LLC
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#


Name:           cups-fax
Version:        3.1.1
Release:        0
Summary:        Virtual PDF/FAX printer for CUPS
License:        GPL-2.0-or-later
Group:          Productivity/Publishing/PDF
URL:            https://www.gtc.de/
Source0:        http://www.gtc.de/src/cups-fax.tar.gz
BuildRequires:  gcc7
BuildRequires:  make
BuildRequires:  cups-devel >= 2.2
BuildRequires:  gsoap-devel >= 2.8
Requires:       cups-client >= 2.2
Requires:       ghostscript >= 9.50
Requires:       python3 >= 3.6

%description
CUPS-FAX is a PDF/FAX writer backend for CUPS.
It provides a virtual CUPS-FAX printer which produces PDF files so that
application programs which have no built-in support to "Send as PDF"
could print to CUPS-FAX to create a PDF file and send it as FAX.
For details see %{_docdir}/cups-fax/README.pdf and %{_docdir}/cups-fax/README.fax
and http://en.opensuse.org/SDB:Printing_to_PDF

%prep
%setup -q -n cups-fax

%build
%global debug_package %nil
cd src
make clean all

%install
install -Dm644 extra/CUPS-FAX_noopt.ppd %{buildroot}%{_datadir}/cups/model/CUPS-FAX_noopt.ppd
install -Dm644 extra/CUPS-FAX_opt.ppd %{buildroot}%{_datadir}/cups/model/CUPS-FAX_opt.ppd
install -Dm640 extra/cups-fax.conf %{buildroot}%{_sysconfdir}/cups/cups-fax.conf
install -Dm700 src/cups-fax %{buildroot}%{_prefix}/lib/cups/backend/cups-fax
install -Dm755 src/faxprint.py %{buildroot}%{_prefix}/lib/cups/backend/faxprint
install -Dm755 src/faxprint.py %{buildroot}%{_prefix}/bin/faxprint
install -dm755 %{buildroot}%{_localstatedir}/spool/cups-fax
pushd %{buildroot}%{_datadir}/cups/model
ln -s CUPS-FAX_opt.ppd CUPS-FAX.ppd
popd
install -Dm644 extra/faxprint.xml %{buildroot}%{_prefix}/share/mime/packages/faxprint.xml
install -Dm644 extra/faxprint.desktop %{buildroot}%{_prefix}/share/applications/faxprint.desktop

%post
# Add a symbolic link if /usr/lib64/cups/backend/ exists:
if test -d %{_libdir}/cups/backend
then
   ln -s %{_prefix}/lib/cups/backend/cups-fax %{_libdir}/cups/backend/cups-fax || :
fi
# Add a "CUPS-FAX" queue if the package is installed (but not when the package is updated):
if test "$1" -eq "1"
then
   %{_sbindir}/lpadmin -h localhost -p CUPS-FAX -v cups-fax:/ -m CUPS-FAX.ppd -E || :
fi
update-mime-database /usr/share/mime
update-desktop-database
xdg-mime default faxprint.desktop application/x-faxprint
# Exit successfully in any case:
exit 0

%postun
# Only if the package is erased (but not when it is replaced with an update package):
if test "$1" -eq "0"
then # Remove the "CUPS-FAX" queue (be silent if it does not exist):
     %{_sbindir}/lpadmin -h localhost -x CUPS-FAX 2>/dev/null || :
     # Remove the symbolic link (ignore if it does not exist):
     rm -f %{_libdir}/cups/backend/cups-fax || :
fi
update-mime-database /usr/share/mime
update-desktop-database
# Exit successfully in any case:
exit 0

%files
%license COPYING
%doc ChangeLog README.pdf README.fax
%config(noreplace) %attr(640, root, lp) %{_sysconfdir}/cups/cups-fax.conf
%dir %{_prefix}/lib/cups
%dir %{_prefix}/lib/cups/backend
%attr(750, root, lp) %{_prefix}/lib/cups/backend/cups-fax
%attr(755, root, lp) %{_prefix}/lib/cups/backend/faxprint
%attr(755, root, lp) %{_prefix}/bin/faxprint
%dir %{_datadir}/cups
%dir %{_datadir}/cups/model
%{_datadir}/cups/model/CUPS-FAX.ppd
%{_datadir}/cups/model/CUPS-FAX_noopt.ppd
%{_datadir}/cups/model/CUPS-FAX_opt.ppd
%dir %{_localstatedir}/spool/cups-fax
%{_prefix}/share/mime/packages/faxprint.xml
%dir %{_prefix}/share/applications
%{_prefix}/share/applications/faxprint.desktop

%changelog
* Tue Jun 09 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.1.1-0
- pappl-main.c: first part for driver v3.
* Fri Apr 24 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.1.0-0
- stub.c: use new parameter 'authmode' for authentication.
* Sat Apr 04 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.9-7
- stub.c: call of command 'login' implemented to get onetime password.
* Tue Mar 10 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.9-6
- copied stdsoap2.c to 208102
* Mon Mar 02 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.9-5
- Makefile stub: fixing version number in stdsoap2.c
* Mon Mar 02 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.9-4
- added missing src/stdsoap2.c
* Thu Feb 19 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.9-3
- README.fax: hints for rendering of images.
* Wed Feb 18 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.9-2
- README.fax: hints, if using AppArmor/SELINUX and Sandboxing of cups.
* Mon Feb 16 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.9-1
- Bugfixes.
* Tue Feb 03 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.8-1
- Bugfixes.
* Mon Feb 02 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.7-1
- Implemented config parameter 'Preview'.
* Tue Jan 20 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.6-1
- Implemented config parameter 'FaxRendering'.
* Mon Jan 19 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.5-1
- Implemented frontend dialog faxprint.py with reading/saving of settings.
* Mon Jan 12 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.4-1
- Read optionally configuration of file in home directory of user.
* Wed Jan 07 2026 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.3-1
- Parameter authuser: changed type to 'value'.
* Thu Dec 11 2025 Jürgen Hammelmann <j.hammelmann@gtc.net> - 3.0.2-1
- Reuse code for PDF/FAX printer.
- Added FAX parameters.
* Tue Jun  8 2021 Tejas Guruswamy <tejas.guruswamy@opensuse.org>
- Add cups-pdf_gs-options.patch to remove .setpdfwrite ghostscript option
  (not present any more from gs 9.54). Fixes boo#1187353.
* Thu Mar 16 2017 joerg.lorenzen@ki.tng.de
- Version update to 3.0.1 which includes:
  - Corrected a bug with multiple instance naming.
  - Some code and logging improvements.
  - Removed obsolete code.
  - Updated the README file.
* Sat Jan 14 2017 joerg.lorenzen@ki.tng.de
- Version update to 3.0.0 which includes:
  - Disabled support for multiple PS-files in one file.
  - Improved PS-structure handling.
  - Reordered compiler call to avoid errors.
* Wed Nov  9 2016 aloisio@gmx.com
- Fixes (boo#984600)
* Wed Apr 27 2016 aloisio@gmx.com
- Spec cleanup
* Thu Nov  6 2014 aloisio@gmx.com
- Version update to 3.0beta2 which includes:
  code simplifications and optimizations
  improved file name handling
  removed contrib/ directory from tarball (now on WWW)
  new contact data
* Fri Jan  3 2014 kieltux@gmail.com
- Version upgrade to 3.0beta1 which includes:
  support for multiple configurations with one backend
  support for option setting via lpoptions or PPD
  new option for selecting output file extension
  improved logging system with better error handling
  various code optimizations
  additional PPD for option setting
  new script in contrib/ for creating user defaults
  updated documentation
* Tue May 15 2012 jsmeix@suse.de
- Version upgrade to 2.6.1 which includes:
  Fixed a non-freed pointer,
  Fixed an invalid line in the config due to a typo.
  Added detailed copyright information to contrib/.
- Version upgrade to 2.6.0 which includes:
  New experimental option for various line delimiters.
  Supplementary groups are set in addition to primary.
* Thu Apr 14 2011 jsmeix@suse.de
- Version upgrade to 2.5.1 which fixes a crash
  due to an uninitialized pointer.
- Added a symbolic link if /usr/lib64/cups/backend/ exists
  to be backward compatible (e.g. for SLE11 systems).
- Use directories like "/etc/" and "/var/" which are fixed
  values in cups-pdf.h also literally in the RPM spec file.
- Install its license in COPYING to be on the safe side.
- Moved the RPM changelog entries to cups-pdf.changes
* Tue Jun 29 2010 jsmeix@suse.de
- Work with upstream compliant CUPS 1.4 on all platforms
  which means to have a fixed "/usr/lib/cups/" directory
  on all platforms (see Novell/Suse Bugzilla bnc#575544).
- Make sure lpadmin talks only to the cupsd on localhost
  via the '-h localhost' command line option.
- Removed the test if /var/run/cups/cups.sock is a socket
  because the local cupsd may run just as well without it
  (only "Listen localhost:631" in cupsd.conf is mandatory).
* Mon Feb  2 2009 suse@irc.freenode.org
- Updated to 2.5.0
* Wed Jan  7 2009 suse@irc.freenode.org
- Initial RPM
