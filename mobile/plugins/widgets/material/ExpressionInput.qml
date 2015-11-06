import Material 0.1
import org.kde.analitza 1.0

TextField
{
    placeholderText: "Expression to calculate..."
    inputMethodHints: /*Qt.ImhPreferNumbers |*/ Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

    Analitza {
        id: a
        variables: app.variables
    }

    property var helperExpression: a.simplify(text)
    hasError: !helperExpression.isCorrect
    helperText: hasError ? helperExpression.toString() : helperExpression.expression

    clip: true
}
