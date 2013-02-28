(defun mapnik-format-function ()
  "Format buffer according to mapnik style (TODO)"
  (add-to-list 'auto-mode-alist '("\\.h\\'" . c++-mode))
  (c-set-style "bsd")
  (c-set-style "bsd")
  (c-set-offset 'innamespace 0)
  (c-set-offset 'template-args-cont 'c-lineup-template-args)
  (setq c-basic-offset 4)
  (indent-region (point-min) (point-max) nil)
  (untabify (point-min) (point-max))
  (delete-trailing-whitespace)
  (save-buffer)
)


