# Test Unity Transform: Local Position vs. World Position

After watching https://youtu.be/ur2ksHAw9SM and https://youtu.be/SZChVvy4enQ and looking at Unity Transform class code:
https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Transform/ScriptBindings/Transform.bindings.cs

As a total beginner concerning C#, I wanted to try it myself because some of my questions did not get answers:
- position = relative to world origin
- local position = relative to parent origin if it exists, else relative to world origin. Said in another way: if there is no parent, then the world is considered as the parent.

But in the Unity C# code, we can see:
```
public extern Vector3 position { get; set; }
public extern Vector3 localPosition { get; set; }
```

Properties do not exist in C++, but in this code (https://stackoverflow.com/a/23109533/8877076), are `localPosition` and `position` two private members? I guess not since the `extern` keyword and Unity is not sharing C++ code, and the C# is a wrapper layer on C++.
