
OUTPUT_DIR = build
SRC = met.tex
SRC_AUX = met.aux

debug: clean

	mkdir -p $(OUTPUT_DIR)
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC)

release: clean

	mkdir -p $(OUTPUT_DIR)
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC) 
	bibtex $(OUTPUT_DIR)/$(SRC_AUX) 
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC) 
	pdflatex -output-directory $(OUTPUT_DIR) $(SRC) 

clean:

	rm -f $(OUTPUT_DIR)/*

mrproper:

	rm -Rf $(OUTPUT_DIR)

distclean: mrproper

run: clean debug

	evince $(OUTPUT_DIR)/met.pdf

release_run: clean release

	evince $(OUTPUT_DIR)/met.pdf

