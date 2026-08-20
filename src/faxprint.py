#!/usr/bin/env python3
import os
import re
import subprocess
import gi
import sys
import time

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk


def list_printers():
    try:
        out = subprocess.check_output(["lpstat", "-p"])
        out = out.decode("utf-8", "replace")
    except Exception as e:
        print("Fehler bei lpstat:", e)
        return []

    printers = []
    for line in out.splitlines():
        line = line.strip()
        m = re.match(r"^(?:printer|Printer|Drucker)\s+([A-Za-z0-9._-]+)", line)
        if m:
            printers.append(m.group(1))

    return printers

def load_config(self):
    home = os.getenv("HOME")
    self.data = {}
    self.data["FaxResolution"] = "NORM"
    self.data["FaxRendering"] = "Grey-Dithered"
    self.data["FaxDelay"] = "1"
    self.data["FaxMaxRetry"] = "1"
    self.data["AuthUser"] = ""
    self.data["AuthPwd"] = ""
    self.data["FaxHeader"] = ""
    self.data["SendingFaxID"] = ""
    self.data["EMailAddress"] = ""
    try:
        with open(f"{home}/.cups-fax.conf", "r") as f:
            for line in f:
                if line and not line.strip().startswith("#"):
                    if " " in line:
                        key, value = line.split(" ", 1)
                        key = key.strip()
                        self.data[key] = value.strip()
    except FileNotFoundError:
        try:
            with open("/etc/cups/cups-fax.conf", "r") as f:
                for line in f:
                    if line and not line.strip().startswith("#"):
                        if " " in line:
                            key, value = line.split(" ", 1)
                            key = key.strip()
                            self.data[key] = value.strip()
        except FileNotFoundError:
            self.data["FaxResolution"] = "NORM"
            self.data["FaxRendering"] = "Grey-Dithered"
            self.data["FaxDelay"] = "1"
            self.data["FaxMaxRetry"] = "1"
            self.data["AuthUser"] = ""
            self.data["AuthPwd"] = ""
            self.data["FaxHeader"] = ""
            self.data["SendingFaxID"] = ""
            self.data["EMailAddress"] = ""
        except PermissionError:
            self.data["FaxResolution"] = "NORM"
            self.data["FaxRendering"] = "Grey-Dithered"
            self.data["FaxDelay"] = "1"
            self.data["FaxMaxRetry"] = "1"
            self.data["AuthUser"] = ""
            self.data["AuthPwd"] = ""
            self.data["FaxHeader"] = ""
            self.data["SendingFaxID"] = ""
            self.data["EMailAddress"] = ""

class App(Gtk.Window):
    def __init__(self, args):
        version = Gtk.get_major_version()
        load_config(self)
        filename = ""
        faxnumber = ""
        if len(args) > 1:
            filename = args[1]
        if len(args) > 2:
            faxnumber = args[2]
        super().__init__(title="FAX/PDF-Print (CUPS)")
        self.set_border_width(12)
        self.set_default_size(360, 160)

        grid = Gtk.Grid(column_spacing=10, row_spacing=10)
        self.add(grid)

        # Printer
        lbl_key = Gtk.Label(label="Printer/Device:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 0, 1, 1)
        self.printer = Gtk.ComboBoxText()
        i = -1
        for p in list_printers():
            self.printer.append_text(p)
            i = i + 1
            if p == "CUPS-FAX":
                self.printer.set_active(i)
        if self.printer.get_active() == -1 and self.printer.get_model() is not None:
            self.printer.set_active(0)
        grid.attach(self.printer, 1, 0, 2, 1)

        # Settings
        self.settings_btn = Gtk.Button.new_from_icon_name("emblem-system-symbolic", Gtk.IconSize.BUTTON)
        self.settings_btn.connect("clicked", self.on_settings)
        grid.attach(self.settings_btn, 3, 0, 1, 1)

        # FaxNumber
        lbl_key = Gtk.Label(label="FAX Number:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 1, 1, 1)
        self.fax_number = Gtk.Entry()
        self.fax_number.set_placeholder_text("+49...")
        self.fax_number.set_text(faxnumber)
        grid.attach(self.fax_number, 1, 1, 2, 1)

        # FaxMaxRetry
        lbl_key = Gtk.Label(label="FAX Max. Retry (0–3):")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 2, 1, 1)
        self.max_retry = Gtk.SpinButton()
        self.max_retry.set_range(0, 3)
        self.max_retry.set_increments(1, 5)
        self.max_retry.set_value(int(self.data["FaxMaxRetry"]))
        grid.attach(self.max_retry, 1, 2, 1, 1)

        # FaxResolution
        lbl_key = Gtk.Label(label="FAX Resolution:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 3, 1, 1)
        self.fax_resolution = Gtk.ComboBoxText()
        i = 0
        for r in ["HIGH", "NORM"]:
            self.fax_resolution.append_text(r)
            if self.data["FaxResolution"].lower() == r.lower():
                self.fax_resolution.set_active(i)
            i = i + 1
        grid.attach(self.fax_resolution, 1, 3, 1, 1)

         # FaxRendering
        lbl_key = Gtk.Label(label="FAX Rendering Levels:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 4, 1, 1)
        self.fax_rendering = Gtk.ComboBoxText()
        i = 0
        for r in ["Black-White", "Grey-Dithered", "Floyd-Steinberg-Dithered", "Ghostscript"]:
            self.fax_rendering.append_text(r)
            if self.data["FaxRendering"].lower() == r.lower():
                self.fax_rendering.set_active(i)
            i = i + 1
        grid.attach(self.fax_rendering, 1, 4, 1, 1)

        # Resolution
        lbl_key = Gtk.Label(label="Resolution:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 5, 1, 1)
        self.resolution = Gtk.ComboBoxText()
        for r in ["204x196", "300x300", "600x600", "1200x1200", "2400x2400"]:
            self.resolution.append_text(r)
        self.resolution.set_active(0)
        grid.attach(self.resolution, 1, 5, 1, 1)

        # Preview
        lbl_key = Gtk.Label(label="FAX Preview via email:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 6, 1, 1)
        self.preview = Gtk.ComboBoxText()
        for r in ["No", "Yes"]:
            self.preview.append_text(r)
        self.preview.set_active(0)
        grid.attach(self.preview, 1, 6, 1, 1)

        # File + Print
        box = Gtk.Box(spacing=10)
        grid.attach(box, 0, 7, 3, 1)

        self.file_btn = Gtk.FileChooserButton(title="Choose file", action=Gtk.FileChooserAction.OPEN)
        self.file_btn.set_filename(filename)
        box.pack_start(self.file_btn, True, True, 0)

        self.print_btn = Gtk.Button(label="Print/Send")
        self.print_btn.connect("clicked", self.on_print)
        box.pack_start(self.print_btn, False, False, 0)

        self.status = Gtk.Label(label="")
        grid.attach(self.status, 0, 8, 3, 1)

    def on_print(self, _btn):
        printer = self.printer.get_active_text()
        path = self.file_btn.get_filename()
        fax_number = self.fax_number.get_text().strip()
        fax_max_retry = int(self.max_retry.get_value())
        fax_resolution = self.fax_resolution.get_active_text()
        fax_rendering = self.fax_rendering.get_active_text()
        resolution = self.resolution.get_active_text()
        preview = self.preview.get_active_text()

        if not printer:
            self.status.set_text("No printer choosen.")
            return
        if not path:
            self.status.set_text("No file choosen.")
            return

        cmd = [
            "lp",
            "-d", printer,
            "-o", f"FaxNumber={fax_number}",
            "-o", f"FaxMaxRetry={fax_max_retry}",
            "-o", f"FaxResolution={fax_resolution}",
            "-o", f"FaxRendering={fax_rendering}",
            "-o", f"Resolution={resolution}",
            "-o", f"Preview={preview}",
            path,
        ]

        try:
            subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            sent = ""
            for c in cmd:
                sent = sent + " " + str(c)
            fax_number = "".join(filter(str.isdigit, fax_number))
            if not fax_number:
                self.status.set_text("Printed!")
            else:
                self.status.set_text("Sent!")
        except subprocess.CalledProcessError as e:
            self.status.set_text(f"Error: {e.output.strip()}")

    def on_settings(self, _btn):
        load_config(self)
        self.settings = Gtk.Window(title="FAX/PDF-Print (Settings)")
        grid = Gtk.Grid(column_spacing=10, row_spacing=10)
        self.settings.add(grid)
        self.settings.set_border_width(12)
        self.settings.set_default_size(280, 180)
        # AuthUser
        # AuthPwd
        # FaxHeader
        # SendingFaxID
        # EMailAddress
        # FaxResolution NORM
        # FaxDelay      1
        # FaxMaxRetry   1
        lbl_key = Gtk.Label(label="Username:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 0, 1, 1)
        self.settings.username = Gtk.Entry()
        self.settings.username.set_text(self.data["AuthUser"])
        grid.attach(self.settings.username, 1, 0, 1, 1)

        lbl_key = Gtk.Label(label="Password:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 1, 1, 1)
        self.settings.password = Gtk.Entry()
        self.settings.password.set_text(self.data["AuthPwd"])
        grid.attach(self.settings.password, 1, 1, 1, 1)

        lbl_key = Gtk.Label(label="FAX Headline:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 2, 1, 1)
        self.settings.header = Gtk.Entry()
        self.settings.header.set_text(self.data["FaxHeader"])
        grid.attach(self.settings.header, 1, 2, 1, 1)

        lbl_key = Gtk.Label(label="FAX Sender ID:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 3, 1, 1)
        self.settings.sendingfaxid = Gtk.Entry()
        self.settings.sendingfaxid.set_text(self.data["SendingFaxID"])
        grid.attach(self.settings.sendingfaxid, 1, 3, 1, 1)

        lbl_key = Gtk.Label(label="E-Mail Address:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 4, 1, 1)
        self.settings.emailaddress = Gtk.Entry()
        self.settings.emailaddress.set_text(self.data["EMailAddress"])
        grid.attach(self.settings.emailaddress, 1, 4, 1, 1)

        lbl_key = Gtk.Label(label="FAX Resolution:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 5, 1, 1)
        self.settings.resolution = Gtk.ComboBoxText()
        i = 0
        for r in ["HIGH", "NORM"]:
            self.settings.resolution.append_text(r)
            if self.data["FaxResolution"].lower() == r.lower():
               self.settings.resolution.set_active(i)
            i = i + 1
        grid.attach(self.settings.resolution, 1, 5, 1, 1)

        # FaxRendering
        lbl_key = Gtk.Label(label="FAX Rendering Levels:")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 6, 1, 1)
        self.settings.rendering = Gtk.ComboBoxText()
        i = 0
        for r in ["Black-White", "Grey-Dithered", "Floyd-Steinberg-Dithered", "Ghostscript"]:
            self.settings.rendering.append_text(r)
            if self.data["FaxRendering"].lower() == r.lower():
                self.settings.rendering.set_active(i)
            i = i + 1
        grid.attach(self.settings.rendering, 1, 6, 1, 1)

        lbl_key = Gtk.Label(label="FAX Delay (0-3):")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 7, 1, 1)
        self.settings.delay = Gtk.SpinButton()
        self.settings.delay.set_range(0, 3)
        self.settings.delay.set_increments(1, 5)
        self.settings.delay.set_value(int(self.data["FaxDelay"]))
        grid.attach(self.settings.delay, 1, 7, 1, 1)

        lbl_key = Gtk.Label(label="FAX Max. Retry (0–3):")
        lbl_key.set_halign(Gtk.Align.START)
        grid.attach(lbl_key, 0, 8, 1, 1)
        self.settings.maxretry = Gtk.SpinButton()
        self.settings.maxretry.set_range(0, 3)
        self.settings.maxretry.set_increments(1, 5)
        self.settings.maxretry.set_value(int(self.data["FaxMaxRetry"]))
        grid.attach(self.settings.maxretry, 1, 8, 1, 1)

        self.settings.save_btn = Gtk.Button(label="Save")
        self.settings.save_btn.connect("clicked", self.on_save)
        grid.attach(self.settings.save_btn, 0, 9, 2, 1)

        self.settings.show_all()

    def on_save(self, _btn):
        self.data["FaxResolution"] = self.settings.resolution.get_active_text()
        self.data["FaxRendering"] = self.settings.rendering.get_active_text()
        self.data["FaxDelay"] = self.settings.delay.get_text()
        self.data["FaxMaxRetry"] = self.settings.maxretry.get_text()
        self.data["AuthUser"] = self.settings.username.get_text()
        self.data["AuthPwd"] = self.settings.password.get_text()
        self.data["FaxHeader"] = self.settings.header.get_text()
        self.data["SendingFaxID"] = self.settings.sendingfaxid.get_text()
        self.data["EMailAddress"] = self.settings.emailaddress.get_text()
        home = os.getenv("HOME")
        with open(f"{home}/.cups-fax.conf", "w") as f:
            for key, value in self.data.items():
                f.write(f"{key} {value}\n")


if __name__ == "__main__":
    win = App(sys.argv)
    win.connect("destroy", Gtk.main_quit)
    win.show_all()
    Gtk.main()

