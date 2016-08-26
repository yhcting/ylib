(load-file "~/.emacs-cedet.el")

(set 'yhc.topdir (concat (getenv "PWD") "/"))
(set 'yhc.target "y")

(defun yhc.compile.clean ()
    "Using make file in top directory"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "cd " yhc.topdir "; make clean; rm -rf bin/; rm -rf lib/; rm -rf include/"))
        (compile cmd)))

(defun yhc.compile.debug ()
    "Using make file in top directory"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "cd " yhc.topdir "; make -j8 install"))
        (compile cmd)))

(defun yhc.compile.release ()
    "Using make file in top directory"
    (interactive)
    (let ((cmd ""))
        (set 'cmd (concat "cd " yhc.topdir "; make -j8 install"))
        (compile cmd)))

(defun yhc.gdb ()
    "gdb wrapper"
    (interactive)
    (if (file-exists-p (concat yhc.topdir "bin/" yhc.target))
        (let ((cmd ""))
            (set 'cmd (concat "gdb -i=mi --annotate=3 " yhc.topdir "bin/" yhc.target))
            ;(message cmd)))
            (gdb cmd))
        (message (concat "target: '" yhc.target "' is not compiled yet."))))

;; override ede's key map for compile...
(global-set-key [f7] 'yhc.compile.clean)
(global-set-key [f8] 'yhc.compile.debug)
(global-set-key [f9] 'yhc.compile.release)
(global-set-key [f5] 'yhc.gdb)

(ede-cpp-root-project (concat yhc.target "-root")
                :name (concat yhc.target "-root")
                :file (concat yhc.topdir "Makefile")
                :include-path '(
                                ;;"../include"
                               )
                :system-include-path '("/usr/include")
                :spp-table '() )
