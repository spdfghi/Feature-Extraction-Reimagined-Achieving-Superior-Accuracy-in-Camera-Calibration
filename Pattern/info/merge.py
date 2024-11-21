from PyPDF2 import PdfMerger

# 创建一个 PdfMerger 对象
merger = PdfMerger()

# 要合并的 PDF 文件列表
# pdf_files = ['star_8.pdf', 'star_16.pdf', 'line_1.pdf', 'line_2.pdf', 'line_3.pdf', 'line_4.pdf']
pdf_files = []
for i in range(8):
    pdf_files.append('8_'+str(i)+".pdf")
pdf_files.append('16.pdf')

# 合并每个 PDF 文件
for pdf in pdf_files:
    merger.append(pdf)

# 输出合并后的 PDF 文件
merger.write('combined.pdf')
merger.close()
