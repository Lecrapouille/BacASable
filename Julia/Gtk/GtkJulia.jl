using Gtk

function julia_main()
  win = GtkWindow("Hello", 400, 200)
  b = GtkButton("button")
  push!(win,b)
  showall(win)
end
