using System.Collections;
using System.Collections.Generic;
using UnityEngine;

// position = relative to world origin
// local position = relative to parent origin if it exists, else relative to world origin => if there is no parent then the world is considered as parent.

// Setter: setPosition(X,Y,Z) == setLocalPosition(X,Y,Z)
// Getter: getPosition(X,Y,Z) == getLocalPosition(X,Y,Z)
public class scr : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        transform.position = new Vector3(1.0f, 0.0f, 0.0f);
        transform.localPosition = new Vector3(3.0f, 0.0f, 0.0f);
	Debug.Log(name + " cst Position: " + transform.position.x + ", " + transform.position.y);
	Debug.Log(name + " cst Local pos: " + transform.localPosition.x + ", " + transform.localPosition.y);
        // Parent: position == local position == (3,0,0)
        // Fils: position == local position == (3,0,0)
    }

    // Update is called once per frame
    void Update()
    {
        transform.position = new Vector3(1.0f, 0.0f, 0.0f);
        transform.localPosition = new Vector3(3.0f, 0.0f, 0.0f);
	Debug.Log(name + " updt Position: " + transform.position.x + ", " + transform.position.y);
	Debug.Log(name + " updt Local pos: " + transform.localPosition.x + ", " + transform.localPosition.y);
        // Parent: position == local position == (3,0,0)
        // Fils: (local position ==  position == (6,0,0)) != (local position == (3,0,0))
    }
}
