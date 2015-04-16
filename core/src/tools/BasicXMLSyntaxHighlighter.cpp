#include "BasicXMLSyntaxHighlighter.h"

BasicXMLSyntaxHighlighter::BasicXMLSyntaxHighlighter(QTextDocument * pTextDoc) :
    QSyntaxHighlighter(pTextDoc),
    m_xmlKeywordFormat(),
    m_xmlElementFormat(),
    m_xmlAttributeFormat(),
    m_xmlValueFormat(),
    m_xmlCommentFormat(),
    m_xmlKeywordRegexes(),
    m_xmlElementRegex("\\b[A-Za-z0-9_]+(?=[\\s/>])"),
    m_xmlAttributeRegex("\\b[A-Za-z0-9_]+(?=\\=)"),
    m_xmlValueStartRegex("\""),
    m_xmlValueEndRegex("\"(?=[\\s></])"),
    m_xmlCommentRegex("<!--[^\n]*-->")
{
    m_xmlKeywordRegexes << QRegExp("\\b?xml\\b")
                        << QRegExp("/>")
                        << QRegExp(">")
                        << QRegExp("<");

    m_xmlKeywordFormat.setForeground(Qt::darkMagenta);
    m_xmlKeywordFormat.setFontWeight(QFont::Bold);

    m_xmlElementFormat.setForeground(Qt::green);
    m_xmlElementFormat.setFontWeight(QFont::Bold);

    m_xmlAttributeFormat.setForeground(Qt::blue);
    m_xmlAttributeFormat.setFontItalic(true);

    m_xmlValueFormat.setForeground(Qt::red);

    m_xmlCommentFormat.setForeground(Qt::gray);
}

void BasicXMLSyntaxHighlighter::highlightBlock(const QString & text)
{
    typedef QList<QRegExp>::const_iterator Iter;
    Iter xmlKeywordRegexesEnd = m_xmlKeywordRegexes.end();
    for(Iter it = m_xmlKeywordRegexes.begin(); it != xmlKeywordRegexesEnd; ++it) {
        const QRegExp & regex = *it;
        highlightByRegex(m_xmlKeywordFormat, regex, text);
    }

    highlightByRegex(m_xmlElementFormat, m_xmlElementRegex, text);
    highlightByRegex(m_xmlAttributeFormat, m_xmlAttributeRegex, text);
    highlightByRegex(m_xmlCommentFormat, m_xmlCommentRegex, text);

    // TODO: highlight xml values specifically
}

void BasicXMLSyntaxHighlighter::highlightByRegex(const QTextCharFormat & format,
                                                 const QRegExp & regex, const QString & text)
{
    int index = regex.indexIn(text);

    while(index >= 0)
    {
        int matchedLength = regex.matchedLength();
        setFormat(index, matchedLength, format);

        index = regex.indexIn(text, index + matchedLength);
    }
}