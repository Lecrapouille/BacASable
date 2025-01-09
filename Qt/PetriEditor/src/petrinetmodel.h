#ifndef PETRINETMODEL_H
#define PETRINETMODEL_H

#include <QObject>
#include <QPointF>
#include <QQmlEngine>

class Arc
{
    Q_GADGET
    Q_PROPERTY(int sourceIndex MEMBER sourceIndex)
    Q_PROPERTY(bool sourceIsPlace MEMBER sourceIsPlace)
    Q_PROPERTY(int targetIndex MEMBER targetIndex)
    Q_PROPERTY(bool targetIsPlace MEMBER targetIsPlace)
public:
    int sourceIndex;
    bool sourceIsPlace;
    int targetIndex;
    bool targetIsPlace;
};
Q_DECLARE_METATYPE(Arc)

class PetriNetModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QPointF> placePositions READ placePositions NOTIFY placePositionsChanged)
    Q_PROPERTY(QList<QPointF> transitionPositions READ transitionPositions NOTIFY transitionPositionsChanged)
    Q_PROPERTY(QVector<Arc> arcs READ arcs NOTIFY arcsChanged)
    Q_PROPERTY(QList<int> tokens READ tokens NOTIFY tokensChanged)

public:
    /// \brief Constructor
    /// \param[in] parent Parent object
    explicit PetriNetModel(QObject *parent = nullptr);

    /// \brief Get list of place positions
    /// \return List of place positions
    QList<QPointF> placePositions() const { return m_placePositions; }

    /// \brief Get list of transition positions
    /// \return List of transition positions
    QList<QPointF> transitionPositions() const { return m_transitionPositions; }

    /// \brief Get list of arcs
    /// \return List of arcs
    QVector<Arc> arcs() const { return m_arcs; }

    QList<int> tokens() const { return m_tokens; }

    Q_INVOKABLE void requestUpdate() {
        emit placePositionsChanged();
        emit transitionPositionsChanged();
        emit arcsChanged();
    }

public slots:
    /// \brief Select a node at given position
    /// \param[in] pos Position to check for node selection
    void selectNodeAt(const QPointF& pos);

    /// \brief Move selected node to new position
    /// \param[in] pos New position for selected node
    void moveSelectedNode(const QPointF& pos);

signals:
    void placePositionsChanged();
    void transitionPositionsChanged();
    void arcsChanged();
    void nodeSelected(int index, bool isPlace);
    void tokensChanged();

private:
    /// \brief List of place positions
    QList<QPointF> m_placePositions;

    /// \brief List of transition positions
    QList<QPointF> m_transitionPositions;

    /// \brief List of arcs
    QVector<Arc> m_arcs;         ///< List of arcs

    /// \brief Index of selected node
    int m_selectedNode;          ///< Index of selected node

    /// \brief True if selected node is a place
    bool m_selectedIsPlace;      ///< True if selected node is a place

    QList<int> m_tokens;         ///< Number of tokens per place
};

#endif // PETRINETMODEL_H