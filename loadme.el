;;;
;;; This is only of interest to EMACS deveopers.
;;; It does the equivalent of aesetenv.ksh for a running emacs.
;;;


(defun trim-trailing-slash (dir)
  (cond ((not (string-equal "" (file-name-nondirectory dir)))
	 dir)
	(t (substring dir 0 (- (length dir) 1)))))

;;; set CURENVBASE
(setenv "CURENVBASE" (trim-trailing-slash (expand-file-name default-directory)))


;;; set path
(setq exec-path (append
		 (list
		  (expand-file-name (concat (getenv "CURENVBASE")
					    "/tools"))
		  (expand-file-name (concat (getenv "CURENVBASE")
					    "/tools/bin")))
		 exec-path))

(setenv "PATH" (format "%s:%s:%s"
		       (expand-file-name (concat (getenv "CURENVBASE")
						 "/tools"))
		       (expand-file-name (concat (getenv "CURENVBASE")
						 "/tools/bin"))
		       (getenv "PATH")))

