# Test Unity Transform: local position vs. world position

After watching https://youtu.be/ur2ksHAw9SM and https://youtu.be/SZChVvy4enQ and looking at Unity Transform class code:
https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Transform/ScriptBindings/Transform.bindings.cs

As a total noob concerning C#, I wanted to give a try by myself because some of my questions did not get answers:
- position = relative to world origin
- local position = relative to parent origin if it exists, else relative to world origin. Said in other way: if there is no parent then the world is considered as parent.

But in the Unity C# code, we can see:
```
public extern Vector3 position { get; set; }
public extern Vector3 localPosition { get; set; }
```

Properties do not exist in C++ but in this code (https://stackoverflow.com/a/23109533/8877076), is `localPosition` and `position` are two private members ? I guess not since the `extern` keyword and Unity is not sharing C++ code and the C# is a wrapper layer on C++.
