(load-file "~/.emacs-cedet.el")

(set 'yhc.topdir (concat (getenv "PWD") "/"))
(set 'yhc.target "y")

(defun yhc.compile.debug ()
    "Using make file in top directory"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "cd " yhc.topdir "; make CFLAGS=-g DEFS=-D_YDBG debug"))
        (compile cmd)))

(defun yhc.compile.release ()
    "Using make file in top directory"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "cd " yhc.topdir "; make release"))
        (compile cmd)))

(defun yhc.gdb ()
    "gdb wrapper"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "gdb --annotate=3 " yhc.topdir yhc.target))
        ;(message cmd)))
        (gdb cmd)))

;; override ede's key map for compile...
(global-set-key [f9]  'yhc.compile.debug)
(global-set-key [f10] 'yhc.compile.release)
(global-set-key [f8]  'yhc.gdb)

(ede-cpp-root-project (concat yhc.target "-root")
                :name (concat yhc.target "-root")
                :file (concat "~/dev/cedetws/" yhc.target "/Makefile")
                :include-path '(
                                ;;"../include"
                               )
                :system-include-path '("/usr/include")
                :spp-table '() )
