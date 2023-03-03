[![Build](https://github.com/TUD-OS/latex-template/actions/workflows/build.yml/badge.svg)](https://github.com/TUD-OS/latex-template/actions/workflows/build.yml)

## Was ist das?

Dieses Diplomarbeits-Latex-Template stellt ein Skelett für eine
Diplomarbeit mit zugehöriger Make-Datei zur Verfügung.

## Checkout Submodules First

Um das `$ make checkbiw` ausführen zu können, musst du zuerst die
Submodule auschecken: `git submodule update --init --recursive`

## So geht's los

Als erstes solltest du in diplom.tex alle Vorkommen von Otto
Mustermann, "Dein Titel" und "Dein Betreuer" ersetzen.

`diplom.tex` will deine Aufgabe als PDF einbinden. Es sucht
`images/diplom-aufgabe.pdf`, was eine A4 Seite sein muss. Mit

    convert <dein-gescanntes-bild> diplom-aufgabe.pdf

solltest du eine beliebige Bilddatei in ein PDF umwandeln können, wenn
ImageMagick auf deinem System installiert ist.

An diesen Punkt sollte `make` ein `diplom.pdf` produzieren.

Das Template unterstützt sowohl englischen und deutschen Text. Englisch ist
standardmäßig eingestellt. Für deutschen Text kann der letzte `\selectlanguage`
Aufruf in `diplom.tex` einfach weggelassen werden.

## How To Build

- `$ make`: regular build that produces `diplom.pdf` and `yyyy-mm-dd_hhmmss DRAFT Diplomarbeit - Branch <branch>.pdf`.
- `$ make watch`: Performs a watch task, i.e. automatically re-builds everything quickly on changes.
   If your PDF viewer supports automatic reload on file changes (such as the default PDF viewer in GNOME)
   you get a cool productive working environment.

### Build Prerequisites (Regular Build)

In case you don't want to install `texlive-full` to save disk space, the following packages are
actually required: \

```shell
$ sudo apt install texlive-base \
   texlive-lang-german \
   texlive-lang-english \
   `# for siunits` \
   texlive-science \
   texlive-luatex \
   `# biber gets invoked by the utilities provided by bibtex-extra` \
   biber \
   texlive-bibtex-extra \
   `# for csquotes` \
   texlive-latex-extra
```

Additionally, please install also:

```shell
$ sudo apt install \
  `# for the `make checkbiw` script`
  diction \
  `# build system to build the latex project`
  latexmk
```

### Build Prerequisites (Nix Build)

This template can also be built using [Nix](https://nixos.org/download.html). You have
the option to either use `$ nix-build`, which will produce `result/diplom.pdf`, or to
use
```bash
$ nix-shell
$ make clean
$ make watch # (or just make)
```

## Grafiken einbinden

Grafiken sollten im `images/` Verzeichnis abgelegt werden und im
Makefile in die entsprechende `DOC_IMG_*` Variable eingetragen
werden. Momentan werden Grafiken in den Formaten PDF, PNG und JPEG
unterstützt. PDF eignet sich für Vektorgrafiken und kann von den
meisten Vektorgrafikprogrammen erzeugt werden (Inkscape, OpenOffice
Draw, ...).

## Tipps

Diese Datei enthält im Moment ein Sammlung von Tipps und Tricks, sowie
einige Hintergrundinformationen.

- passive voice: **do not use it**
  - There is a Makefile template checking for 'Bugs in Writing' according
    to the book of the same name (`make checkbiw`). Diction must be installed
    somewhere in the path (check out the directory
    `checkbiw/diction` for details).
  - Vim users can add detection for passive voice and *weasel words* via
    Björn's [`vim-weasel` package](https://github.com/bjoernd/vim-weasel)
- font sizes in images: adapt to other text size
   (ideally, use PGF/TikZ and PGFPlots)
- avoid missing meta data in PDF files (title, keywords, author)
- "good" title page
- use biblatex for references, it pays off fast
- convert images to correct include types (vector formats, e.g. PDF)
- protected spaces between, e.g., `Figure~1`, `~\cite{xyz}`
- units: use the `siunitx` package to typeset units
- French spacing: tell latex what is an end of sentence with `\@.`
  where it cannot know it (e.g., `This is a sentence ending on an
  abbreviation BASIC\@.  Next sentence.`)
  - Again, you can try to detect a good portion of French spacing
    using an automatic algorithm (`make check-french-spacing`).
    Improvements are welcome.
- listings with at least three elements have a
  comma before the last and/or (*serial* or *Oxford comma*):
  *"Set A contains elements a, b, and c."*
- more stylistic information can be found in *Bugs in writing* (BIW)
  by Lyn Dupré
- add chapters without a chapter number (e.g. appendix) with `\addchap{Chapter without Number}`
  instead of `\chapter*{Chapter without Number}`. Otherwise, `\chaptername` and similar macros
  inherit the name of the last chapter with a number in several cases.
  `\addchap` is a macro provided by KOMA
- this project uses a `.editorconfig` which makes sure you have the same basic formatting
  settings across IDEs **as you type**. Support exists in Clion, VS Code, IntelliJ,
  vim, ... make sure to activate the setting or add the plugin for that into your IDE/editor


## spezielle Tipps von Frank

- Ich verwende in der Vorlage KOMA-Script (`scrbook`), welches vor allem
  für den deutschsprachigen Raum gedacht ist. KOMA-Script kann auch
  international verwendet werden, das Format ist aber für
  Englischsprachige Arbeiten etwas unüblich.

- Ein Hinweis zum Erstellen der Grafiken: Viele verwenden xfig, ich
  habe meine Grafiken mit OpenOffice Draw erzeugt. Dort hat man mehr
  Möglichkeiten. Einfach als PDF exportieren. Damit alle Grafiken
  eine gleichmäßige Größe besitzen, habe ich einfach immer die
  Seitengröße so gesetzt, dass die Zeichnung voll erfasst wird. Dann
  habe ich immer die gleiche Schriftgröße gesetzt. Beim Einbinden der
  Grafiken ins LaTeX-File habe ich einen Faktor, z.B.

        \includegraphics[width=190\figurewidth]{architecture}

  mit eingebaut. Den Faktor setze ich dann einfach am Anfang mit

        \setlength{\figurewidth}{.070cm}

  und bin damit in der Lage, alle Grafiken zugleich in der Größe zu verändern.
  Die Zahl 190 aus dem includegraphics-Statement kommt von der gewählten
  Seitengröße in OpenOffice Draw (190mm).

- Wenn man viele Grafiken hat, die man genau ausrichten will, ist ein

        \usepackage{placeins}

  sinnvoll. Dann kann man am Ende einer Seite

        \FloatBarrier

  schreiben und erzwingt die Ausgabe aller noch offenen Grafiken an
  diesem Punkt. Mir ist bewusst, dass das etwas unschön ist, aber
  manchmal braucht man das wirklich, z.B. wenn man alle Messwerte auf
  einer Seite unterbringen will.
