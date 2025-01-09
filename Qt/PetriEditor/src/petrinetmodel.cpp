#include "petrinetmodel.h"
#include <QRectF>
#include <QDebug>

PetriNetModel::PetriNetModel(QObject *parent)
    : QObject(parent)
    , m_selectedNode(-1)
    , m_selectedIsPlace(false)
{
    // Create a simple Petri net: P1 -> T1 -> P2
    m_placePositions << QPointF(100, 150)  // P1
                     << QPointF(300, 150);  // P2

    m_transitionPositions << QPointF(200, 150);  // T1

    // Initialize tokens (example: P1 has 1 token, P2 has 0 tokens)
    m_tokens << 1 << 0;

    // Add arcs: P1 -> T1 and T1 -> P2
    Arc arc1{0, true, 0, false};   // P1 -> T1
    Arc arc2{0, false, 1, true};   // T1 -> P2
    m_arcs << arc1 << arc2;

    // Debug log for arcs
    qDebug() << "Arcs created:";
    for (const Arc& arc : m_arcs) {
        qDebug() << "Arc:"
                 << (arc.sourceIsPlace ? "P" : "T") << arc.sourceIndex
                 << "->"
                 << (arc.targetIsPlace ? "P" : "T") << arc.targetIndex;
    }
}

void PetriNetModel::selectNodeAt(const QPointF& pos)
{
    const qreal nodeRadius = 20;
    qDebug() << "Selecting node at" << pos;

    // Check places
    for (int i = 0; i < m_placePositions.size(); ++i) {
        QRectF bounds(m_placePositions[i].x() - nodeRadius,
                     m_placePositions[i].y() - nodeRadius,
                     2 * nodeRadius,
                     2 * nodeRadius);
        if (bounds.contains(pos)) {
            m_selectedNode = i;
            m_selectedIsPlace = true;
            qDebug() << "Selected place" << i;
            emit nodeSelected(i, true);
            return;
        }
    }

    // Check transitions
    for (int i = 0; i < m_transitionPositions.size(); ++i) {
        QRectF bounds(m_transitionPositions[i].x() - nodeRadius,
                     m_transitionPositions[i].y() - nodeRadius,
                     2 * nodeRadius,
                     2 * nodeRadius);
        if (bounds.contains(pos)) {
            m_selectedNode = i;
            m_selectedIsPlace = false;
            qDebug() << "Selected transition" << i;
            emit nodeSelected(i, false);
            return;
        }
    }

    // No node selected
    qDebug() << "No node selected";
    m_selectedNode = -1;
    emit nodeSelected(-1, false);
}

void PetriNetModel::moveSelectedNode(const QPointF& pos)
{
    if (m_selectedNode == -1) {
        return;
    }

    // Check if position has actually changed
    QPointF& currentPos = m_selectedIsPlace ?
        m_placePositions[m_selectedNode] :
        m_transitionPositions[m_selectedNode];

    if (currentPos == pos) {
        return;  // No change, no need to emit signal
    }

    // Update position
    currentPos = pos;

    // Emit appropriate signal
    if (m_selectedIsPlace) {
        emit placePositionsChanged();
    } else {
        emit transitionPositionsChanged();
    }
}