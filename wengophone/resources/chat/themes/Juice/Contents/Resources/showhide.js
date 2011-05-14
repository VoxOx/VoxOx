function handleParent(parentId)
{
        myParent = document.getElementById("menu" + parentId)

        if (myParent.style.display=="none") {
                myParent.style.display="block"
        } else {
                myParent.style.display="none"
        }
}