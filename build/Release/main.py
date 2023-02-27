import cv2
import pytesseract
from fpdf import FPDF

pytesseract.pytesseract.tesseract_cmd = 'C:\\Program Files\\Tesseract-OCR\\tesseract.exe'

img = cv2.imread('output.jpg')
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

outputString = str(pytesseract.image_to_string(img))  # converts to string

# print(outputString)

wordsList = outputString.splitlines()

print(wordsList)

pdf = FPDF()

# Add a page to the PDF
pdf.add_page()

# Set the font and size of the text
pdf.set_font("Arial", size=12)

selected = -1000

# Loop through each line of the text
for i, line in enumerate(outputString.split("\n")):
    # Choose the alignment based on the line number
    if i == selected:
        align = "L"
    else:
        align = "C"

    # Write the line to the PDF cell
    pdf.cell(0, 10, line, 0, 1, align)

# Save the PDF with the specified file name
pdf.output("output.pdf")
