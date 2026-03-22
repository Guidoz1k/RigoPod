template via https://github.com/schnorr/infufrgs
with the modification to infufrgs.cls

\newenvironment{agradecimentos}{%
  \chapter*{Agradecimentos}%
}{}

became

\newenvironment{acknowledgements}{%
  \chapter*{Acknowledgements}%
}{}

so Acknowledgements would appear in english






latex on vscode via https://mathjiajia.github.io/vscode-and-latex/

important: in mathjiajia's text you find instructions for path defining in windows 10 (https://www.computerhope.com/issues/ch000549.htm)

Bibliography not being recognized nor rendered (while using latex on vscode):

In the Tex section of the Latex-Workshop extension (in the most left vertical panel) it should appear the 'commands' section. Inside that section, it should appear de option 'Build Latex Projects' -> 'Recipe: pdflatex-bibtex-pdflatex x2'.

If you click on that it should rebuild the bibtex database using the last .bib file, and that should solve your problem. The problem occurs because the default pdflatex building motor in VS it not automatically update de .bib file to its bibtex database; and with that problem although it can recognizes the cites in the IDE, it cannot put them in the built document.





using glossary package
After compiling once, run in the terminal: makeglossaries main
Then compile again
that should solve the glossary in a more elegant way, always in alphabetical order

