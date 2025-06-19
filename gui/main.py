import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Vte', '2.91')
gi.require_version('Gdk', '3.0')

from gi.repository import Gtk, Vte, GLib, Gdk

import subprocess
import time

def rgba(hex_str):
    c = Gdk.RGBA()
    c.parse(hex_str)
    return c

def on_spawned(term, task, user_data):
    term.feed_child(b"source activate.sh\n")
    term.feed_child(b"clear\n")
    pass

def spawn_terminal(terminal):
    terminal.spawn_async(
        Vte.PtyFlags.DEFAULT,
        None,
        ["/bin/bash"],
        None,
        GLib.SpawnFlags.DEFAULT,
        None, None,
        -1,
        None,
        on_spawned
    )
    palette = [
            rgba('#073642'),  # black
            rgba('#dc322f'),  # red
            rgba('#859900'),  # green
            rgba('#b58900'),  # yellow
            rgba('#268bd2'),  # blue
            rgba('#d33682'),  # magenta
            rgba('#2aa198'),  # cyan
            rgba('#eee8d5'),  # white
            rgba('#002b36'),  # bright black (base02)
            rgba('#cb4b16'),  # bright red
            rgba('#586e75'),  # bright green
            rgba('#657b83'),  # bright yellow
            rgba('#839496'),  # bright blue
            rgba('#6c71c4'),  # bright magenta
            rgba('#93a1a1'),  # bright cyan
            rgba('#fdf6e3'),  # bright white (base3)
        ]
    terminal.set_colors( rgba("#fcfcfc"),rgba("#232627"),palette)
    

# Ventana principal
win = Gtk.Window(title="Terminal embebida")
win.set_default_size(900, 500)

# Layout horizontal
hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=5)
win.add(hbox)

# Panel de botones (izquierda)
button_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=5)



button1 = Gtk.Button(label="Activate")
button2 = Gtk.Button(label="Limpiar")
btn_mnt_remote = Gtk.Button(label="Mount")
btn_copy_remote = Gtk.Button(label="Copy")
btn_tty_remote = Gtk.Button(label="R tty")
btn_build_aarch64_remote = Gtk.Button(label="Build aarch64")
btn_build_x86_64_remote = Gtk.Button(label="Build x86_64_amd64")
btn_build_alpine_remote = Gtk.Button(label="Build x86_64_musl")
btn_build_this = Gtk.Button(label="Build")
btn_run_musl = Gtk.Button(label="RUN (docker)")
btn_run_remote = Gtk.Button(label="RUN (remote)")


button_box.pack_start(button1, False, False, 0)
button_box.pack_start(button2, False, False, 0)
button_box.pack_start(btn_mnt_remote, False, False, 0)
button_box.pack_start(btn_copy_remote, False, False, 0)
button_box.pack_start(btn_tty_remote, False, False, 0)
button_box.pack_start(btn_build_aarch64_remote, False, False, 0)
button_box.pack_start(btn_build_x86_64_remote, False, False, 0)
button_box.pack_start(btn_build_alpine_remote, False, False, 0)
button_box.pack_start(btn_build_this, False, False, 0)
button_box.pack_start(btn_run_musl, False, False, 0)
button_box.pack_start(btn_run_remote, False, False, 0)

# Terminal
term = Vte.Terminal()
spawn_terminal(term)

def on_btn_mnt_remote(btn):
    result = subprocess.run(
        "bash -c 'source activate.sh && ./scripts/rpi_mount.sh'",
        shell=True,
        capture_output=True,
        text=True
    )
    # print(str(result.stdout))
    if(result.returncode == 1):
        btn.set_label("Umount")
    else:
        btn.set_label("Mount")
    
    if(result.stdout):
        print(str(result.stdout))
    if(result.stderr):
        print(str(result.stderr))


def on_btn_copy_remote(btn):
    term.feed_child(b"./scripts/rpi_copy.sh\n")
    return
    result = subprocess.run(
        "bash -c 'source activate.sh && ./scripts/rpi_copy.sh'",
        shell=True,
        capture_output=True,
        text=True
    )
     
    if(result.stdout):
        print(str(result.stdout))
    if(result.stderr):
        print(str(result.stderr))

def on_btn_tty_remote(btn):
    import subprocess

    subprocess.Popen([
        "konsole", "-e",
        "bash", "-i", "-c", "source activate.sh && ./scripts/rpi_tty.sh"
    ])

    pass

def on_btn_run_remote(btn):
    term.feed_child(b"source activate.sh && ./scripts/rpi_copy.sh && ssh -t $REMOTE_SSH_ARGS $REMOTE_SSH\n")
    term.feed_child(b"(cd $REMOTE_WORK  && $HH_CORE_EXEC $HH_CORE_EXEC_ARG) ; exit \n")


    return
    import subprocess

    subprocess.Popen([
        "konsole", "-e",
        "bash", "-i", "-c", "source activate.sh && ./scripts/rpi_run.sh"
    ])

    pass



# Agrega al layout
hbox.pack_start(button_box, False, False, 5)
hbox.pack_start(term, True, True, 5)

button1.connect("clicked", lambda b: term.feed_child(b"source activate.sh\n"))
button2.connect("clicked", lambda b: term.feed_child(b"clear\n"))
btn_mnt_remote.connect("clicked", on_btn_mnt_remote)
btn_copy_remote.connect("clicked", on_btn_copy_remote)
btn_tty_remote.connect("clicked", on_btn_tty_remote)
btn_build_aarch64_remote.connect("clicked", lambda b: term.feed_child(b"./scripts/build.sh aarch64\n"))
btn_build_x86_64_remote.connect("clicked", lambda b: term.feed_child(b"./scripts/build.sh x86_64_amd64\n"))
btn_build_alpine_remote.connect("clicked", lambda b: term.feed_child(b"./scripts/build.sh musl\n"))
btn_build_this.connect("clicked", lambda b: term.feed_child(b"./scripts/build.sh\n"))
btn_run_musl.connect("clicked", lambda b: term.feed_child(b"./scripts/run_docker_musl.sh\n"))
btn_run_remote.connect("clicked", on_btn_run_remote)


# Eventos
win.connect("destroy", Gtk.main_quit)
win.show_all()
Gtk.main()
