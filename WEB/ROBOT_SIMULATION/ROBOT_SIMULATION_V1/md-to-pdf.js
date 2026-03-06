const fs = require('fs');
const { marked } = require('marked');
const path = require('path');

const mdContent = fs.readFileSync(path.join(__dirname, 'DOCUMENT.md'), 'utf-8');
const htmlBody = marked.parse(mdContent);

const html = `<!DOCTYPE html>
<html lang="ko">
<head>
<meta charset="UTF-8">
<title>Co-Robot Sewing Simulation - 기술 문서</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Noto+Sans+KR:wght@300;400;500;700&family=Fira+Code:wght@400;500&display=swap');

  * { margin: 0; padding: 0; box-sizing: border-box; }

  body {
    font-family: 'Noto Sans KR', sans-serif;
    font-size: 11pt;
    line-height: 1.7;
    color: #1a1a2e;
    padding: 40px 50px;
    max-width: 900px;
    margin: 0 auto;
  }

  h1 {
    font-size: 22pt;
    font-weight: 700;
    color: #0f3460;
    border-bottom: 3px solid #0f3460;
    padding-bottom: 10px;
    margin-bottom: 24px;
    margin-top: 0;
  }

  h2 {
    font-size: 16pt;
    font-weight: 700;
    color: #16213e;
    border-bottom: 2px solid #e94560;
    padding-bottom: 6px;
    margin-top: 32px;
    margin-bottom: 16px;
    page-break-after: avoid;
  }

  h3 {
    font-size: 13pt;
    font-weight: 600;
    color: #0f3460;
    margin-top: 24px;
    margin-bottom: 12px;
    page-break-after: avoid;
  }

  h4 {
    font-size: 11pt;
    font-weight: 600;
    color: #533483;
    margin-top: 18px;
    margin-bottom: 8px;
    page-break-after: avoid;
  }

  p {
    margin-bottom: 10px;
    text-align: justify;
  }

  code {
    font-family: 'Fira Code', 'Consolas', monospace;
    background: #f0f0f5;
    padding: 2px 6px;
    border-radius: 4px;
    font-size: 9.5pt;
    color: #e94560;
  }

  pre {
    background: #1a1a2e;
    color: #e0e0e0;
    padding: 16px 20px;
    border-radius: 8px;
    overflow-x: auto;
    margin: 12px 0 16px;
    font-size: 9pt;
    line-height: 1.5;
    page-break-inside: avoid;
  }

  pre code {
    background: none;
    color: inherit;
    padding: 0;
    font-size: 9pt;
  }

  table {
    width: 100%;
    border-collapse: collapse;
    margin: 12px 0 16px;
    font-size: 10pt;
    page-break-inside: avoid;
  }

  th {
    background: #0f3460;
    color: white;
    font-weight: 500;
    padding: 8px 12px;
    text-align: left;
    border: 1px solid #0f3460;
  }

  td {
    padding: 7px 12px;
    border: 1px solid #ddd;
  }

  tr:nth-child(even) td {
    background: #f8f9fa;
  }

  tr:hover td {
    background: #e8f0fe;
  }

  ul, ol {
    margin: 8px 0 12px 24px;
  }

  li {
    margin-bottom: 4px;
  }

  strong {
    color: #16213e;
  }

  hr {
    border: none;
    height: 2px;
    background: linear-gradient(to right, #0f3460, #e94560, #0f3460);
    margin: 30px 0;
  }

  blockquote {
    border-left: 4px solid #e94560;
    padding: 8px 16px;
    margin: 12px 0;
    background: #fef5f5;
    color: #333;
  }

  @media print {
    body { padding: 20px 30px; }
    pre { page-break-inside: avoid; }
    table { page-break-inside: avoid; }
    h2, h3, h4 { page-break-after: avoid; }
  }
</style>
</head>
<body>
${htmlBody}
</body>
</html>`;

const outputPath = path.join(__dirname, 'DOCUMENT.html');
fs.writeFileSync(outputPath, html, 'utf-8');
console.log('HTML generated: ' + outputPath);