;;; dip-mode.el --- Syntax highlighter for DIPL. -*- coding: utf-8; lexical-binding: t; -*-

;; Copyright © 2026, by Ondrej Pego Jaura

;; Author: Ondrej Pego Jaura 
;; Version: 1.0.0
;; Created: 14. Sept. 2026
;; Keywords: languages, DIP
;; Homepage: https://github.com/vrtulka23/scinumtools3

;; This file is not part of GNU Emacs.

;;; License:

;; You can redistribute this program and/or modify it under the terms of the GNU General Public License version 2.

;;; Commentary:

;; Syntax highlighter for DIP serialization language

(setq dip-font-lock-keywords
      (let* (
            ;; define several category of keywords
            (x-keywords '("true" "false" "none"))
            (x-types '("float" "int" "bool" "str" "table" "float32" "float64" "float128" "int16" "int32" "int64" "uint16" "uint32" "uint64"))
            (x-constants '("$source" "$unit" "$schema"))
            (x-events '("!options" "!constant" "!format" "!condition" "!tags" "!delimiter"))
            (x-metadata '("?descr" "?authors" "?title" "?journal" "?year" "?volume" "?issue" "?pages" "?doi" "?url" "?version" "?created" "?modified" "?license"))
            (x-functions '("@if" "@elif" "@else" "@end"))

            ;; generate regex string for each category of keywords
            (x-keywords-regexp (regexp-opt x-keywords 'words))
            (x-types-regexp (regexp-opt x-types 'words))
            (x-constants-regexp (regexp-opt x-constants 'signs))
            (x-events-regexp (regexp-opt x-events 'signs))
            (x-metadata-regexp (regexp-opt x-metadata 'signs))
            (x-functions-regexp (regexp-opt x-functions 'signs)))

        `(
          (,x-types-regexp . 'font-lock-type-face)
          (,x-constants-regexp . 'font-lock-constant-face)
          (,x-events-regexp . 'dip-directive-face)
          (,x-metadata-regexp . 'dip-metadata-face)
          (,x-functions-regexp . 'font-lock-function-name-face)
          (,x-keywords-regexp . 'font-lock-keyword-face)
          ("{{[^}\n]*}\\(?:\\[[^]\n]*\\]\\)?}" (0 'dip-reference-face prepend))
          ("{[^{}\n]*}\\[\\([0-9:., ]*\\)\\]"
           (0 'dip-reference-face prepend)
           (1 'dip-slice-face prepend))
          ("{[^{}\n]*}\\(?:\\[[^]\n]*\\)?" (0 'dip-reference-face prepend))
          ("#[^\n]*" . 'font-lock-comment-face)
          ;; note: order above matters, because once colored, that part won't change.
          ;; in general, put longer words first
          )))

(defface dip-metadata-face
  '((t :foreground "#4f8585"))
  "Subdued face for DIPL provenance metadata.")

(defface dip-directive-face
  '((t :foreground "#806a58"))
  "Subdued face for DIPL property directives.")

(defface dip-reference-face
  '((t :foreground "#a66f43"))
  "Subtle face for DIPL references and template substitutions.")

(defface dip-slice-face
  '((t :foreground "#c18a5d"))
  "Subtle face for DIPL reference slice indices.")

(defface my-number-face
  '((t :foreground "#d47b7b"))
  "Face for highlighting numbers")

;; Highlight numbers and scientific notation in all programming modes
(defun highlight-dip-mode-numbers ()
  "Highlight numbers and scientific notation in all programming modes."
  (font-lock-add-keywords
   nil
   '(("-?\\b[0-9]+\\b\\(?:\\.[0-9]*\\(?:[eE][-+]?[0-9]+\\)?\\)?\\b\\.?" . 'my-number-face))))

;; Add the function to dip modes hooks
(add-hook 'dip-mode-hook 'highlight-dip-mode-numbers)

(defvar dip-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?{ "." st)
    (modify-syntax-entry ?} "." st)
    st)
  "Syntax table for dip-mode")

;;;###autoload
(define-derived-mode dip-mode fundamental-mode "DIPL"
  "Major mode for editing DIPL source files."

  ;; code for syntax highlighting
  (setq font-lock-defaults '((dip-font-lock-keywords))))

;; add the mode to the `features' list
(provide 'dip-mode)

;;; dip-mode.el ends here
