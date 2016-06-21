/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

function selectForm(){
    if(document.forms['create-re']['re-type'][0].checked){
        $("#ints").removeClass("hide");
        $("#dates").addClass("hide");
        $(".floats-field").addClass("hide");
        document.getElementById("max-label").innerHTML = 'Maximum digits (leave blank for no limit):';
        $("#phone-number").addClass("hide");
    }else if(document.forms['create-re']['re-type'][1].checked){
        $("#ints").addClass("hide");
        $("#dates").removeClass("hide");
        $("#phone-number").addClass("hide");
    }else if(document.forms['create-re']['re-type'][2].checked){
        $("#ints").removeClass("hide");
        $("#dates").addClass("hide");
        $(".floats-field").removeClass("hide");
        document.getElementById("max-label").innerHTML = 'Max digits before decimal (leave blank for no limit):';
        $("#phone-number").addClass("hide");
    }else if(document.forms['create-re']['re-type'][3].checked){
        $("#ints").addClass('hide');
        $("#dates").addClass("hide");
        $("#phone-number").removeClass("hide");
    }
}

function createRegularExpression(){
    var expression = "";
    
    if(document.forms['create-re']['re-type'][0].checked){
        expression = createRegularExpressionInt();
    }else if(document.forms['create-re']['re-type'][1].checked){
        expression = createRegularExpressionDate();
    }else if(document.forms['create-re']['re-type'][2].checked){
        expression = createRegularExpressionFloat();
    }else if(document.forms['create-re']['re-type'][3].checked){
        expression = createRegularExpressionPhone();
    }
    
    document.getElementById("re").innerHTML = expression;
}

function createRegularExpressionDate(){
    var expression = "";
    var months;
    var days;
    var years;
    var sep = "([";
    var customSeps = document.forms['create-re']['custom-sep-input'].value;
    
    if(!document.forms['create-re']['foreward-slash-sep'].checked
            && !document.forms['create-re']['period-sep'].checked
            && !document.forms['create-re']['custom-sep-check'].checked){
        expression = "Please select a separator.";
        return expression;
    }
    
    months = "1[0-2]";
    days = "3[0-1]|[1-2][0-9]";
    if(document.forms['create-re']['enforce-zero'].checked){
        months += "|0[1-9]";
        days += "|0[1-9]";
    }else{
        months += "|0?[1-9]";
        days += "|0?[1-9]";
    }

    if(document.forms['create-re']['year-selection'][0].checked){
        years = "\\d{2}";
    }else if(document.forms['create-re']['year-selection'][1].checked){
        years = "\\d{4}";
    }else{
        years = "\\d{2}|\\d{4}";
    }
    
    if(document.forms['create-re']['group-labels'].checked){
        months = "(?P&ltmonth&gt" + months + ")";
        days = "(?P&ltday&gt" + days + ")";
        years = "(?P&ltyear&gt" + years + ")";
    }else{
        months = "(" + months + ")";
        days = "(" + days + ")";
        years = "(" + years + ")";
    }
    
    if(document.forms['create-re']['foreward-slash-sep'].checked){
        sep += "/";
    }
    
    if(document.forms['create-re']['period-sep'].checked){
        sep += ".";
    }
    
    if(document.forms['create-re']['custom-sep-check'].checked){
        for(var i = 0; i < customSeps.length; i++){
            if(customSeps[i] == '^' || customSeps[i] == '\\' || customSeps[i] == '-'){
                sep += "\\" + customSeps[i];
            }else{
                sep += customSeps[i];
            }
        }
    }
    
    sep += "])";
    
    if(document.forms['create-re']['edit-format'][0].checked){
        expression = months + sep + days + "(\\2)" + years;//mdy
    }else{
        expression = days + sep + months + "(\\2)" + years;//dmy
    }
    
    return expression;
}

function createRegularExpressionInt(){
    var maxDigitsStr = document.forms["create-re"]["Max_dig"].value;
    var maxDigits;
    var isLimit;
    var leadingZeroes;

    var commaSeparatorList = document.forms["create-re"]["comma-separator"];
    var normAllowed;
    var commaAllowed;

    var zeroExpr = "";
    var negExpr = "";

    var normExpr = "";

    var numThreeGroups;
    var remainder;
    var commaRemExpr = "";
    var commaExpr = "";
    var commaGroup = "(,[0-9]{3})";

    var expression = "";

    // Checking for limit
    if(maxDigitsStr != '') {
      maxDigits = parseInt(maxDigitsStr);
      if (maxDigits < 1) {
        return "Number of digits must be at least 1";
      }
      isLimit = true;
    }
    else {
      isLimit = false;
    }

    // Determine if norm (no commas) and/or commas are allowed.  If maxDigits is 3 or less, commas
    // don't make sense.
    if (isLimit && maxDigits <= 3) {
      normAllowed = true;
      commaAllowed = false;
    }
    else if (commaSeparatorList[0].checked) {
      normAllowed = true;
      commaAllowed = false;
    }
    else if (commaSeparatorList[1].checked) {
      normAllowed = true;
      commaAllowed = true;
    }
    else if (commaSeparatorList[2].checked) {
      normAllowed = false;
      commaAllowed = true;
    }

    // if(document.forms["create-re"]["negative"].checked){
    //     if(document.forms['create-re']['neg-zero'].checked){
    //         zeroExpres = "(-?0)"expression += "((-?)0|(-?)";
    //     }else{
    //         expression += "(0|(-?)";
    //     }
    // }else{
    //     expression += "(0|";
    // }
    
    // Create zero string and special negative lookahead assertion for leading zeros without negative zero string
    if(document.forms["create-re"]["leadingZeroes"].checked){
      leadingZeroes = true;
    } else {
      leadingZeroes = false;
      if(document.forms["create-re"]["negative"].checked && document.forms['create-re']['neg-zero'].checked) {
        zeroExpr = "-?0";
      }
      else {
        zeroExpr = "0";
      }
    }
    
    // Create negative expression
    if(document.forms["create-re"]["negative"].checked){
      if (leadingZeroes && !document.forms['create-re']['neg-zero'].checked) {
	negExpr = "(-(?=[0,]*[1-9]))?";
      }
      else {
        negExpr = "-?";
      }
    }

    // Generate normal expression
    if(normAllowed) {
      if(leadingZeroes) {
	if(isLimit) {
	  normExpr = "[0-9]{1," + maxDigits + "}";
	}
	else {
	  normExpr = "[0-9]+";
	}
      }
      else {
	if(isLimit) {
	  if (maxDigits != 1) {
	    normExpr = "[1-9][0-9]{0," + (maxDigits - 1) + "}";
	  }
	  else {
	    normExpr = "[1-9]";
	  }
	}
	else {
	  normExpr = "[1-9][0-9]*";
	}
      }
    }
    
    // Create comma expressions
    if (commaAllowed) {

      if (isLimit) {
        numThreeGroups = Math.floor(maxDigits / 3);
        remainder = maxDigits % 3;

	// Create remainder expression if remainder is 1 or 2
        if (remainder == 1) {
	  if (leadingZeroes) {
	    commaRemExpr = "[0-9]" + commaGroup + "{" + numThreeGroups + "}";
	  }
	  else {
	    commaRemExpr = "[1-9]" + commaGroup + "{" + numThreeGroups + "}";
	  }
	}
	else if (remainder == 2) {
	  if (leadingZeroes) {
	    commaRemExpr = "[0-9]{1,2}" + commaGroup + "{" + numThreeGroups + "}";
	  }
	  else {
	    commaRemExpr = "[1-9][0-9]?" + commaGroup + "{" + numThreeGroups + "}";
	  }
	}

	// Create comma expression
	if (leadingZeroes) {
	  if (numThreeGroups == 1) { 
	    commaExpr = "[0-9]{1,3}";
	  }
	  else {
	    commaExpr = "[0-9]{1,3}" + commaGroup + "{0," + (numThreeGroups - 1) + "}";
	  }
	}
	else {
	  if (numThreeGroups == 1) { 
	    commaExpr = "[1-9][0-9]{0,2}";
	  }
	  else {
	    commaExpr = "[1-9][0-9]{0,2}" + commaGroup + "{0," + (numThreeGroups - 1) + "}";
	  }
	}
      }
      else { // no limit
	if (leadingZeroes) {
	  commaExpr = "[0-9]{1,3}" + commaGroup + "*";
	}
	else {
	  commaExpr = "[1-9][0-9]{0,2}" + commaGroup + "*";
	}
      }
    }

    // Add negative indicators
    if (negExpr != "") {
      if (normExpr != "") {
        normExpr = negExpr + normExpr;
      }
      if (commaExpr != "") {
        commaExpr = negExpr + commaExpr;
      }
      if (commaRemExpr != "") {
        commaRemExpr = negExpr + commaRemExpr;
      }
    }

    //if(maxDigits != '' && !isNaN(maxDigits)){
    //    maxDigits = parseInt(maxDigits) - 1;
    //    if(maxDigits !== 0){
    //        unformattedMaxDigits = "[0-9]{0," + maxDigits + "}"; //with limit
    //    }else{
    //        unformattedMaxDigits = "";
    //    }
    //    if(maxDigits % 3 > 0){
    //        formattedMaxDigitsOverflow = "[0-9]{0," + (maxDigits % 3) + "}";
    //    }else{
    //        formattedMaxDigitsOverflow = "";
    //    }
    //    if(parseInt(maxDigits / 3) > 0)
    //        formattedMaxDigitsGroups = "(,[0-9]{3}){0," + parseInt(maxDigits / 3) + "}";
    //    else
    //        formattedMaxDigitsGroups = "";
    //} // if nothing, no limit
    
    //if(commaSeparatorList[2].checked){  // REQUIRED
    //    expression += "[" + leadingZeroes + "-9]" + formattedMaxDigitsOverflow;
    //    expression += "" + formattedMaxDigitsGroups + ")";
    //}else if(commaSeparatorList[0].checked){  // NOT ALLOWED
    //    expression += "[" + leadingZeroes + "-9]" + unformattedMaxDigits + ")";
    //}else{  // ALLOWED BUT NOT REQUIRED
    //    expression += "(([" + leadingZeroes + "-9]" + unformattedMaxDigits + ")|";
    //    expression += "([" + leadingZeroes + "-9]" + formattedMaxDigitsOverflow;
    //    expression += "" + formattedMaxDigitsGroups + ")))";
   // }
    
    // Construct expression
    if (normAllowed && commaAllowed) {
      expression = normExpr + "|" + commaExpr;
    }
    else if (normAllowed) {
      expression = normExpr;
    }
    else {
      expression = commaExpr;
    }
    if (zeroExpr != "") {
      expression = zeroExpr + "|" + expression;
    }
    if (commaRemExpr != "") {
      expression = expression + "|" + commaRemExpr;
    }

    return expression;
    
}

function createRegularExpressionFloat(){
    var maxDigits = document.forms["create-re"]["Max_dig"].value;
    var maxPostDigits = document.forms['create-re']['Max_dig_post_dec'].value;
    var expression = "";
    var trailingZeroes = "";
    var maxDigitsPostDec = "[0-9]*";
    
    if((maxDigits != '' && isNaN(maxDigits)) || (maxPostDigits != '' && isNaN(maxPostDigits))){
        expression = "Maximum Digits Before and After Decimal must be numerical.";//error
        return expression;
    }
    
    expression = createRegularExpressionInt(); //builds pre-decimal portion
    
    if(document.forms['create-re']['trailingZeroes'].checked){
        trailingZeroes = "0"
    }else{
        trailingZeroes = '1';
    }
    
    if(maxPostDigits != '' && !isNaN(maxPostDigits)){
        maxPostDigits = parseInt(maxPostDigits) - 1;
        if(maxPostDigits !== 0){
            maxDigitsPostDec = "[0-9]{0," + maxPostDigits + "}";
        }else{
            maxDigitsPostDec = "";
        }
    }
    
    expression += "([.])(0|" + maxDigitsPostDec + "[" + trailingZeroes + "-9])";
    
    return expression;
    
}

function createRegularExpressionPhone(){
    var expression = "";
    var multipleFormats = false;
    var areaCode = document.forms["create-re"]["area-code"].value;
    
    if(!document.forms["create-re"]["full-format"].checked
            && !document.forms["create-re"]["dash-format"].checked
            && !document.forms["create-re"]["unformatted"].checked){
        expression = "You must select a format.";
        return expression;
    }
    
    if(areaCode.length != 3 || isNaN(areaCode)){
        areaCode = "[0-9]{3}";
    }
    
    if(document.forms['create-re']['full-format'].checked){
        multipleFormats = true;
        expression += "(([(]" + areaCode + "[)])([ ])([0-9]{3})([-])([0-9]{4}))";
    }
    
    if(document.forms["create-re"]["dash-format"].checked){
        if(multipleFormats){
            expression += "|";
        }else{
            multipleFormats = true;
        }
        expression += "((" + areaCode + ")([-])([0-9]{3})([-])([0-9]{4}))";
    }
    
    if(document.forms["create-re"]["unformatted"].checked){
        if(multipleFormats){
            expression += "|";
        }
        expression += "((" + areaCode + ")([0-9]{3})([0-9]{4}))";
    }
    return expression;
}



function swapNegZero(){
    if(document.forms["create-re"]["negative"].checked){
        $("#neg-zero-wrapper").removeClass('hide');//insert toggle class visible
    }else{
        $("#neg-zero-wrapper").addClass('hide');//insert toggle class invisible
    }
}
