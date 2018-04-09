/***************************************************************************
* Copyright (C) 2014 by Renaud Guezennec                                   *
* http://www.rolisteam.org/contact                      *
*                                                                          *
*  This file is part of DiceParser                                         *
*                                                                          *
* DiceParser is free software; you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by     *
* the Free Software Foundation; either version 2 of the License, or        *
* (at your option) any later version.                                      *
*                                                                          *
* This program is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
* GNU General Public License for more details.                             *
*                                                                          *
* You should have received a copy of the GNU General Public License        *
* along with this program; if not, write to the                            *
* Free Software Foundation, Inc.,                                          *
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.                 *
***************************************************************************/

#include <QStringList>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QGuiApplication>

#include "displaytoolbox.h"
#include "diceparser.h"
#include "highlightdice.h"

/**
 * @page Dice
 * The cli for DiceParser the new dice system from rolisteam.
 * @section Build and install
 * To build this program, type these command:
 * - mkdir build
 * - cd build
 * - cmake ../
 * - make
 * - make install
 * @return
 */

QTextStream out(stdout, QIODevice::WriteOnly);
bool markdown = false;
enum EXPORTFORMAT {TERMINAL, SVG, IMAGE, MARKDOWN, JSON, BOT};


QString diceToMarkdown(QJsonArray array,bool withColor,bool allSameColor,bool allSameFaceCount )
{
    if(allSameFaceCount)
    {
        QStringList result;
        for(auto item : array)
        {
            auto obj = item.toObject();
            auto values= obj["values"].toArray();
            for(auto val : values)
            {
                result.append(val.toString());
            }
        }
        return result.join(',');
    }
    else
    {
        QStringList result;
        for(auto item : array)
        {
            QStringList subResult;
            auto obj = item.toObject();
            auto values= obj["values"].toArray();
            for(auto val : values)
            {
                subResult.append(val.toString());
            }
            result.append(QStringLiteral("d%1:(").arg(obj["face"].toString()));
            result.append(subResult.join(','));
            result.append(QStringLiteral(")"));

        }
        return result.join(' ');
    }
}

void displayImage(QString scalarText, QString resultStr,QJsonArray array, bool withColor, QString cmd, QString comment, bool allSameFaceCount,bool allSameColor)
{
    out << DisplayToolBox::makeImage( scalarText,  resultStr, array,  withColor,  cmd,  comment,  allSameFaceCount, allSameColor);
}
void displayJSon(QString scalarText, QString resultStr,QJsonArray array, bool withColor, QString cmd, QString error, QString comment, bool allSameFaceCount,bool allSameColor)
{
    Q_UNUSED(withColor);
    QJsonDocument doc;
    QJsonObject obj;
    obj["values"]=array;
    obj["comment"]=comment;
    obj["error"]=error;
    obj["scalar"]=scalarText;
    obj["string"]=resultStr;
    obj["allSameFace"]=allSameFaceCount;
    obj["allSameColor"]=allSameColor;
    obj["command"]=cmd;
    doc.setObject(obj);
    out << doc.toJson() << "\n";
}
void displayMarkdown(QString scalarText, QString resultStr,QJsonArray array, bool withColor, QString cmd, QString error, QString comment, bool allSameFaceCount,bool allSameColor)
{
    Q_UNUSED(withColor);
    QString str("```Markdown\n");
    if(!error.isEmpty())
    {
        str.append(QStringLiteral("Error: %1\n").arg(error));
    }
    else
    {
        if(!comment.isEmpty())
        {
            str.prepend(QStringLiteral("%1\n").arg(comment));
        }
        auto diceList = DisplayToolBox::diceToText(array,false,allSameFaceCount,allSameColor);
        if(resultStr.isEmpty())
        {
           str.append(QStringLiteral("# %1\nDetails:[%3 (%2)]\n").arg(scalarText).arg(diceList).arg(cmd));
        }
        else if(!resultStr.isEmpty())
        {
            resultStr.replace("%2",diceList.trimmed());
            str.append(QStringLiteral("%1\n").arg(resultStr));
        }
    }
    str.append(QStringLiteral("```"));
    out << str;
}
void displaySVG(QString scalarText, QString resultStr,QJsonArray array, bool withColor, QString cmd, QString error, QString comment, bool allSameFaceCount,bool allSameColor)
{
    QString str("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<svg version=\"1.1\"  xmlns=\"http://www.w3.org/2000/svg\" "
                                  "xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");
    if(!error.isEmpty())
    {
        str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"20\"><tspan fill=\"red\">%1</tspan></text>").arg(error));
    }
    else
    {
        int y = 20;
        if(!comment.isEmpty())
        {
            str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"%2\"><tspan fill=\"blue\">%1</tspan></text>").arg(comment).arg(y));
            y+=20;
        }
        auto diceList = DisplayToolBox::diceToSvg(array,withColor,allSameColor,allSameFaceCount);
        if(resultStr.isEmpty())
        {
            if(withColor)
                str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"%4\"><tspan fill=\"red\">%1</tspan> details:[%3 (%2)]</text>").arg(scalarText).arg(diceList).arg(cmd).arg(y));
            else
                str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"%4\"><tspan>%1</tspan> details:[%3 (%2)]</text>").arg(scalarText).arg(diceList).arg(cmd).arg(y));
        }
        else if(!resultStr.isEmpty())
        {
            resultStr.replace("%2",diceList.trimmed());
            str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"%2\">%1</text>").arg(resultStr).arg(y));
        }
    }
    str.append(QStringLiteral("</svg>\n"));
    out << str << "\n";
}

void displayCommandResult(QString scalarText, QString resultStr,QJsonArray array, bool withColor, QString cmd, QString error, QString comment, bool allSameFaceCount,bool allSameColor)
{
    if(!error.isEmpty())
    {
        out << "Error" << error << "\n";
        return;
    }
    QString str;

    auto diceList = DisplayToolBox::diceToText(array,withColor,allSameFaceCount,allSameColor);

    if(withColor)
        str = QString("Result: \e[0;31m%1\e[0m - details:[%3 (%2)]").arg(scalarText).arg(diceList).arg(cmd);
    else
        str = QString("Result: %1 - details:[%3 (%2)]").arg(scalarText).arg(diceList).arg(cmd);

    if(!resultStr.isEmpty())
    {
        resultStr.replace("%2",diceList.trimmed());
        str = resultStr;
    }

    if(!comment.isEmpty())
    {
        out << "\033[1m" << comment << "\033[0m\n";
    }
    out << str << "\n";
}

int startDiceParsing(QStringList& cmds,QString& treeFile,bool withColor, EXPORTFORMAT format)
{
    DiceParser parser;
    parser.insertAlias(new DiceAlias("L5R5R",QStringLiteral("L[-,⨀,⨀⬢,❂⬢,❁,❁⬢]")),0);
    parser.insertAlias(new DiceAlias("L5R5S",QStringLiteral("L[-,-,⨀,⨀,⨀❁,⨀⬢,⨀⬢,❂,❂⬢,❁,❁,❁]")),1);
    int rt=0;
    for(QString cmd : cmds)
    {
        if(parser.parseLine(cmd))
        {
            parser.start();
            QList<ExportedDiceResult> list;
            bool homogeneous = true;
            parser.getLastDiceResult(list,homogeneous);
            bool allSameFaceCount, allSameColor;
            auto array =  DisplayToolBox::diceToJson(list,allSameFaceCount,allSameColor);
            QString resultStr;
            QString scalarText;
            QString lastScalarText;
            QString comment = parser.getComment();
            QString error = parser.humanReadableError();
            QStringList strLst;

            if(parser.hasIntegerResultNotInFirst())
            {
                auto values = parser.getLastIntegerResults();
                for(auto val : values )
                {
                    strLst << QString::number(val);
                }
                scalarText = QString("%1").arg(strLst.join(','));
                lastScalarText = strLst.last();
            }
            else if(!list.isEmpty())
            {
                auto values = parser.getSumOfDiceResult();
                for(auto val : values )
                {
                    strLst << QString::number(val);
                }
                scalarText = QString("%1").arg(strLst.join(','));
            }

            if(parser.hasStringResult())
            {
                bool ok;
                QStringList allStringlist = parser.getAllStringResult(ok);
                QString stringResult = allStringlist.join(" ; ");
				// fixme: change semantic of %1
                stringResult.replace("%1",scalarText);
				// fixme: change semantic of %3
                stringResult.replace("%3",lastScalarText);

                int i = 1;
                for(auto value : strLst)
                {
					// todo: check that this block is effectivly replacing the $i variable
					if (i < strLst.size())
					{
						stringResult.replace(QStringLiteral("$%1").arg(i),value);
						++i;
					}
                }

                resultStr = stringResult;
            }
            if(format == BOT)
            {
                if(allSameColor)
                {
                    format = MARKDOWN;
                }
                else
                {
                    format = IMAGE;
                }
                if(!error.isEmpty())
                {
                    format = MARKDOWN;
                }
            }

            switch(format)
            {
                case TERMINAL:
                    displayCommandResult(scalarText, resultStr, array, withColor, cmd, error, comment, allSameFaceCount, allSameColor);
                break;
                case SVG:
                    displaySVG(scalarText, resultStr, array, withColor, cmd, error, comment, allSameFaceCount, allSameColor);
                break;
                case MARKDOWN:
                    displayMarkdown(scalarText, resultStr, array, withColor, cmd, error, comment, allSameFaceCount, allSameColor);
                break;
                case JSON:
                    displayJSon(scalarText, resultStr, array, withColor, cmd, error, comment, allSameFaceCount, allSameColor);
                break;
                case IMAGE:
                    displayImage(scalarText, resultStr, array, withColor, cmd, comment, allSameFaceCount, allSameColor);
                break;
            }
            if(!treeFile.isEmpty())
            {
                parser.writeDownDotTree(treeFile);
            }

            if(!error.isEmpty())
            {
                rt = 1;
            }
        }
        else
        {
            rt = 1;
        }
    }

    return rt;
}
#include <QTextCodec>
int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    QStringList commands;
    QString cmd;
    QString dotFileStr;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    bool colorb=true;
    out.setCodec("UTF-8");
    EXPORTFORMAT format = TERMINAL;

    QCommandLineParser optionParser;
    QCommandLineOption color(QStringList() << "c"<< "color-off", "Disable color to highlight result");
    QCommandLineOption version(QStringList() << "v"<< "version", "Show the version and quit.");
    QCommandLineOption reset(QStringList() << "reset-settings", "Erase the settings and use the default parameters");
    QCommandLineOption alias(QStringList() << "a" << "alias", "path to alias json files: <aliasfile>","aliasfile");
    QCommandLineOption character(QStringList() << "s" << "charactersheet", "set Parameters to simulate character sheet: <sheetfile>","sheetfile");
    QCommandLineOption markdown(QStringList() << "m" <<"markdown", "The output is formatted in markdown.");
    QCommandLineOption bot(QStringList() << "b" <<"bot", "Discord bot.");
    QCommandLineOption svg(QStringList() << "g" <<"svg", "The output is formatted in svg.");
    QCommandLineOption json(QStringList() << "j" <<"json", "The output is formatted in json.");
    QCommandLineOption dotFile(QStringList() << "d"<<"dot-file", "Instead of rolling dice, generate the execution tree and write it in <dotfile>","dotfile");
    QCommandLineOption translation(QStringList() << "t"<<"translation", "path to the translation file: <translationfile>","translationfile");
    QCommandLineOption help(QStringList() << "h"<<"help", "Display this help");

    optionParser.addOption(color);
    optionParser.addOption(version);
    optionParser.addOption(reset);
    optionParser.addOption(dotFile);
    optionParser.addOption(alias);
    optionParser.addOption(character);
    optionParser.addOption(markdown);
    optionParser.addOption(bot);
    optionParser.addOption(svg);
    optionParser.addOption(json);
    optionParser.addOption(translation);
    optionParser.addOption(help);
    for(int i=0;i<argc;++i)
    {
        commands << QString::fromUtf8(argv[i]);
    }

    optionParser.process(commands);

    if(optionParser.isSet(color))
    {
        commands.removeAt(0);
        colorb = false;
    }
    else if(optionParser.isSet(version))
    {
        out << "Rolisteam DiceParser v1.0.0"<< "\n";
        out << "More Details: www.rolisteam.org"<< "\n";
        return 0;
    }
    else if(optionParser.isSet(reset))
    {
        return 0;
    }
    else if(optionParser.isSet(dotFile))
    {
        dotFileStr = optionParser.value(dotFile);
    }
    if(optionParser.isSet(markdown))
    {
        format= MARKDOWN;
    }
    else if(optionParser.isSet(bot))
    {
        format = BOT;
    }
    else if(optionParser.isSet(svg))
    {
        format = SVG;
    }
    else if(optionParser.isSet(json))
    {
        format = JSON;
    }
    if(optionParser.isSet(help))
    {
        cmd = "help";
    }
    QStringList cmdList = optionParser.positionalArguments();

    QString aliasstr;
    if(optionParser.isSet(alias))
    {
        aliasstr = optionParser.value(alias);
    }
    if(optionParser.isSet(help))
    {
        out << optionParser.helpText();
    }
    return startDiceParsing(cmdList,dotFileStr,colorb,format);
}
