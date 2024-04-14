/*************************************************************************************
 *  Copyright (C) 2007-2009 by Aleix Pol <aleixpol@kde.org>                          *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "functionedit.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <KColorScheme>

#include <analitza/analyzer.h>
#include <analitza/expression.h>
#include <analitza/value.h>
#include <analitza/variables.h>
#include <analitzagui/algebrahighlighter.h>
#include <analitzagui/expressionedit.h>
#include <analitzagui/plotsview2d.h>
#include <analitzaplot/planecurve.h>
#include <analitzaplot/plotsfactory.h>
#include <analitzaplot/plotsmodel.h>
#include <klocalizedstring.h>

using namespace Analitza;

namespace
{
static const int resolution = 200;
}

FunctionEdit::FunctionEdit(QWidget *parent)
    : QWidget(parent)
    , m_calcUplimit(0)
    , m_calcDownlimit(0)
    , m_modmode(false)
{
    setWindowTitle(i18n("Add/Edit a function"));

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(2, 2, 2, 2);
    topLayout->setSpacing(5);

    m_name = new QLineEdit(this);

    m_func = new ExpressionEdit(this);
    m_func->setExamples(PlotsFactory::self()->examples(Dim2D));
    m_func->setAns(QStringLiteral("x"));
    connect(m_func, &QPlainTextEdit::textChanged, this, &FunctionEdit::edit);
    connect(m_func, &ExpressionEdit::returnPressed, this, &FunctionEdit::ok);

    m_valid = new QLabel(this);
    m_valid->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    QPalette p = palette();
    p.setColor(QPalette::Active, QPalette::Base, Qt::white);
    m_valid->setPalette(p);

    m_validIcon = new QLabel(this);
    m_validIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLayout *validLayout = new QHBoxLayout;
    validLayout->addWidget(m_validIcon);
    validLayout->addWidget(m_valid);

    m_color = new KColorCombo(this);
    m_color->setColor(QColor(0, 150, 0));
    connect(m_color, SIGNAL(currentIndexChanged(int)), this, SLOT(colorChange(int)));

    m_funcsModel = new PlotsModel(this);
    m_funcsModel->setResolution(resolution);

    m_viewTabs = new QTabWidget(this);

    m_graph = new PlotsView2D(m_viewTabs);
    m_graph->setModel(m_funcsModel);
    m_graph->setViewport(QRectF(QPointF(-10.0, 10.0), QSizeF(20.0, -20.0)));
    m_graph->setFocusPolicy(Qt::NoFocus);
    m_graph->setMouseTracking(false);
    m_graph->setFramed(true);
    m_graph->setReadOnly(true);
    m_graph->setTicksShown(Qt::Orientation(0));

    m_viewTabs->addTab(m_graph, QIcon::fromTheme(QStringLiteral("document-preview")), i18n("Preview"));
    QWidget *options = new QWidget(m_viewTabs);
    options->setLayout(new QVBoxLayout);
    m_uplimit = new ExpressionEdit(options);
    m_downlimit = new ExpressionEdit(options);
    m_uplimit->setText(QStringLiteral("2*pi"));
    m_downlimit->setText(QStringLiteral("0"));
    options->layout()->addWidget(new QLabel(i18n("From:"), options));
    options->layout()->addWidget(m_downlimit);
    options->layout()->addWidget(new QLabel(i18n("To:"), options));
    options->layout()->addWidget(m_uplimit);
    options->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_viewTabs->addTab(options, QIcon::fromTheme(QStringLiteral("configure")), i18n("Options"));
    connect(m_uplimit, &QPlainTextEdit::textChanged, this, &FunctionEdit::updateUplimit);
    connect(m_downlimit, &QPlainTextEdit::textChanged, this, &FunctionEdit::updateDownlimit);

    QHBoxLayout *m_butts = new QHBoxLayout;
    m_ok = new QPushButton(i18n("OK"), this);
    m_ok->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
    m_remove = new QPushButton(i18nc("@action:button", "Remove"), this);
    m_remove->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    connect(m_ok, &QAbstractButton::clicked, this, &FunctionEdit::ok);
    connect(m_remove, &QAbstractButton::clicked, this, &FunctionEdit::removeEditingPlot);

    topLayout->addWidget(m_name);
    topLayout->addWidget(m_func);
    topLayout->addWidget(m_color);
    topLayout->addLayout(validLayout);
    topLayout->addWidget(m_viewTabs);
    topLayout->addLayout(m_butts);

    m_name->hide(); // FIXME: Remove this when the name has any sense

    m_butts->addWidget(m_ok);
    m_butts->addWidget(m_remove);

    m_func->setFocus();
    m_ok->setEnabled(false);

    QFont errorFont = m_valid->font();
    errorFont.setBold(true);
    m_valid->setFont(errorFont);
}

FunctionEdit::~FunctionEdit()
{
}

void FunctionEdit::clear()
{
    m_func->setText(QString());
    m_funcsModel->clear();
    edit();
}

void FunctionEdit::setFunction(const QString &newText)
{
    m_func->setText(newText);
    m_func->document()->setModified(true);
}

void FunctionEdit::setColor(const QColor &newColor)
{
    m_color->setColor(newColor);
    if (m_funcsModel->rowCount() > 0)
        m_funcsModel->setData(m_funcsModel->index(0), newColor);
}

void FunctionEdit::colorChange(int)
{
    setColor(m_color->color());
}

static double calcExp(const Analitza::Expression &exp, const QSharedPointer<Analitza::Variables> &v, bool *corr)
{
    Q_ASSERT(exp.isCorrect());
    Analitza::Analyzer d(v);
    d.setExpression(exp);
    Analitza::Expression r = d.calculate();

    *corr = r.isCorrect() && r.isReal();

    if (*corr)
        return r.toReal().value();
    else
        return 0.;
}

void FunctionEdit::updateUplimit()
{
    bool corr = m_uplimit->isCorrect();
    if (corr) {
        Analitza::Expression e = m_uplimit->expression();
        m_calcUplimit = calcExp(e, m_vars, &corr);
        m_uplimit->setCorrect(corr);
        if (corr)
            edit();
    }
}

void FunctionEdit::updateDownlimit()
{
    bool corr = m_downlimit->isCorrect();
    if (corr) {
        Analitza::Expression e = m_downlimit->expression();
        m_calcDownlimit = calcExp(e, m_vars, &corr);
        m_downlimit->setCorrect(corr);
        if (corr)
            edit();
    }
}

void FunctionEdit::setState(const QString &text, bool negative)
{
    QFontMetrics fm(m_valid->font());
    m_valid->setText(fm.elidedText(text, Qt::ElideRight, m_valid->width()));
    m_valid->setToolTip(text);

    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role = negative ? KColorScheme::NegativeText : KColorScheme::PositiveText;

    QPalette p = m_valid->palette();
    p.setColor(foregroundRole(), scheme.foreground(role).color());
    m_valid->setPalette(p);

    if (negative)
        m_validIcon->setPixmap(QIcon::fromTheme(QStringLiteral("flag-red")).pixmap(QSize(16, 16)));
    else
        m_validIcon->setPixmap(QIcon::fromTheme(QStringLiteral("flag-green")).pixmap(QSize(16, 16)));
}

/// Let's see if the exp is correct
void FunctionEdit::edit()
{
    if (m_func->text().isEmpty()) {
        m_func->setCorrect(true);
        m_ok->setEnabled(false);
        m_valid->clear();
        m_valid->setToolTip(QString());
        m_validIcon->setPixmap(QIcon::fromTheme(QStringLiteral("flag-yellow")).pixmap(QSize(16, 16)));

        m_funcsModel->clear();
        m_graph->forceRepaint();
        return;
    }

    if (!m_uplimit->isCorrect() || !m_downlimit->isCorrect()) {
        setState(i18n("The options you specified are not correct"), true);
        return;
    }

    if (m_calcDownlimit > m_calcUplimit) {
        setState(i18n("Downlimit cannot be greater than uplimit"), true);
        return;
    }
    bool added = false;

    PlaneCurve *f = nullptr;
    PlotBuilder req = PlotsFactory::self()->requestPlot(expression(), Dim2D, m_vars);
    if (req.canDraw())
        f = createFunction();

    if (f && f->isCorrect())
        f->update(QRect(-10, 10, 20, -20));

    m_funcsModel->clear();
    if (f && f->isCorrect()) {
        m_funcsModel->addPlot(f);
        added = true;
        setState(QStringLiteral("%1:=%2").arg(m_name->text(), f->expression().toString()), false);
    } else {
        QStringList errors = req.errors();
        if (f)
            errors = f->errors();
        Q_ASSERT(!errors.isEmpty());

        setState(errors.first(), true);
        m_valid->setToolTip(errors.join(QStringLiteral("<br />")));
        delete f;
    }
    m_func->setCorrect(added);
    m_ok->setEnabled(added);
}

void FunctionEdit::ok()
{
    if (m_ok->isEnabled())
        Q_EMIT accept();
}

void FunctionEdit::focusInEvent(QFocusEvent *)
{
    m_func->setFocus();
}

PlaneCurve *FunctionEdit::createFunction() const
{
    PlotBuilder req = PlotsFactory::self()->requestPlot(expression(), Dim2D, m_vars);
    PlaneCurve *curve = static_cast<PlaneCurve *>(req.create(color(), name()));
    curve->setResolution(resolution);
    if (m_calcUplimit != m_calcDownlimit) {
        const auto parameters = curve->parameters();
        for (const QString &var : parameters)
            curve->setInterval(var, m_calcUplimit, m_calcDownlimit);
    }
    return curve;
}

Analitza::Expression FunctionEdit::expression() const
{
    return m_func->expression();
}

void FunctionEdit::setOptionsShown(bool shown)
{
    m_viewTabs->setVisible(shown);
}

void FunctionEdit::resizeEvent(QResizeEvent *)
{
    QFontMetrics fm(m_valid->font());
    m_valid->setText(fm.elidedText(m_valid->toolTip(), Qt::ElideRight, m_valid->width()));
}

void FunctionEdit::setEditing(bool m)
{
    m_modmode = m;
    m_remove->setVisible(m);
}

#include "moc_functionedit.cpp"
