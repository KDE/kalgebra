/*
 *   Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.analitza 1.0

Item {
	property Component compactRepresentation: Component {
		PlasmaComponents.Button {
			iconSource: "kalgebra"
			onClicked: plasmoid.togglePopup()
		}
	}
	Analitza {
		id: analitza
	}
	
	PlasmaComponents.TextField {
		id: input
		focus: true
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}
		onAccepted: {
			var exp = analitza.execute(text)
			if(exp.isCorrect) {
				view.state = "result"
				view.text = exp.expression
			} else {
				view.state = "error"
				view.text = i18n("Errors: %1", exp.errors)
			}
		}
		onTextChanged: {
			var exp = analitza.simplify(text);
			if(exp.isCorrect) {
				view.state = "simplification"
				view.text = exp.expression
			}
		}
	}
	
	PlasmaComponents.Label {
		id: view
		anchors {
			left: parent.left
			right: parent.right
			bottom: parent.bottom
			top: input.bottom
		}
		
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
		text: i18n("Enter some expression")
		wrapMode: Text.WordWrap
		font.pixelSize: theme.defaultFont.mSize.height

		onStateChanged: { adaptTextSize() }
		onTextChanged: { adaptTextSize() }
		onHeightChanged: { adaptTextSize() }
		onWidthChanged: { adaptTextSize() }

		function adaptTextSize() {
			var isResult = (state == "result");
			resizeTimer.running = isResult;

			if(!isResult) {
				font.pixelSize = theme.defaultFont.mSize.height
				return;
			}
		}
		Timer {
			id: resizeTimer
			interval: 100
			repeat: false

			onTriggered: {
				view.font.pointSize = height
				while(view.paintedHeight>view.height || view.paintedWidth>view.width) {
					view.font.pointSize--;
				}
			}
		}

		states: [
			State {
				name: "result"
				PropertyChanges { target: view; font.bold: true }
			},
			State {
				name: "error"
				PropertyChanges { target: view; color: "red" }
			},
			State { name: "simplification" }
		]
	}
}
